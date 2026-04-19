# Assignment 6: Improvisation

For the final assignment, we set out to turn our operating system into something that feels like a real, interactive system rather than a collection of isolated components. The goal was to build features that tie together everything from the previous assignments — keyboard interrupts, PIT timing, memory management, VGA output, and the PC speaker — into a cohesive user experience.

We implemented five features:

1. An input system with blocking keyboard reads
2. An interactive command shell with system information display
3. A typing speed test
4. A starfield simulator (visual effect)
5. A menu system that connects everything together


## 6.1 Input System

Up until this point, our keyboard handler in `isr.c` could only echo characters to the screen as they arrived through IRQ1. There was no way for user-level code to say "wait here until the user presses a key and give me what they typed." This is a fundamental requirement for any interactive feature, so building a proper input layer was the first priority.

We created a ring buffer that sits between the IRQ handler and the rest of the system. When IRQ1 fires and a key is pressed, the handler translates the scancode to ASCII (using our existing lookup tables from Assignment 3) and pushes the character into the buffer via `input_put_char()`. User code calls `getchar()` to pull characters out the other end.

### Placing the buffer at a fixed physical address

Our kernel is compiled with `-fPIE` (position-independent executable), which means the compiler generates code that accesses global variables through the GOT (Global Offset Table). In a normal hosted environment, the dynamic linker populates the GOT at load time. But in our bare-metal kernel, there is no dynamic linker. We discovered during testing that this caused a problem: the interrupt handler and normal code were resolving the same variable name to different physical addresses. The interrupt handler would write a character into the buffer, but `getchar()` would never see it because it was reading from a different memory location.

The solution was to place the ring buffer at a fixed, known physical address instead of relying on the compiler's variable placement. We chose address `0x80000`, which sits in conventional memory below our kernel heap and is unused at this stage of boot:

```c
#define RING_BASE       0x80000
#define RING_DATA       ((volatile char     *)RING_BASE)
#define RING_WRITE_PTR  ((volatile uint16_t *)(RING_BASE + INPUT_BUFFER_SIZE))
#define RING_READ_PTR   ((volatile uint16_t *)(RING_BASE + INPUT_BUFFER_SIZE + 2))
```

By casting fixed integer addresses to `volatile` pointers, both `input_put_char()` (called from the interrupt handler) and `getchar()` (called from normal code) access the exact same physical memory, regardless of how the compiler resolves GOT entries. The `volatile` qualifier ensures the compiler generates actual load/store instructions on every access, preventing it from caching values in registers across iterations.

This approach is similar to how we access the VGA text buffer at `0xB8000` — that also works by casting a known physical address to a pointer, which is why VGA output never had this problem.

### Blocking input with getchar()

The `getchar()` function enables interrupts with `sti` and then spins until the write pointer differs from the read pointer:

```c
char getchar(void)
{
    __asm__ volatile("sti");

    while (*RING_READ_PTR == *RING_WRITE_PTR) {
        /* spin — interrupts are enabled so IRQ1 will fire */
    }

    uint16_t rd = *RING_READ_PTR;
    char c = RING_DATA[rd];
    *RING_READ_PTR = (rd + 1) % INPUT_BUFFER_SIZE;
    return c;
}
```

The `sti` instruction re-enables hardware interrupts. Without it, IRQ1 would never fire and no keystrokes would arrive. While the CPU spins in the loop, keyboard interrupts arrive, the ISR runs `input_put_char()` which advances `RING_WRITE_PTR`, and the loop condition becomes false.

### Line input and shift key support

We built `getline()` on top of `getchar()`, which reads characters until Enter is pressed, supports backspace, and echoes typed characters to the screen. This function is used by the shell for command input and by the typing test for word entry.

We also modified the IRQ1 handler in `isr.c` to support shift keys, so that the user can type uppercase letters and symbols. The handler tracks whether either shift key is currently held down and selects the appropriate scancode lookup table:

```c
static int shift_held = 0;

/* Inside irq_handler, IRQ1 branch: */
if (scancode == 0x2A || scancode == 0x36)       /* shift pressed  */
    shift_held = 1;
else if (scancode == 0xAA || scancode == 0xB6)  /* shift released */
    shift_held = 0;
else if ((scancode & 0x80) == 0) {               /* key press      */
    uint8_t ascii;
    if (shift_held)
        ascii = scancode2ascii_shift[scancode];
    else
        ascii = scancode2ascii[scancode];
    /* ... */
    input_put_char((char)ascii);
}
```


## 6.2 Interactive Shell

The shell is the centrepiece of our improvisation. It is a command-line interpreter where the user types commands and the OS executes them. This single feature integrates every major component from every previous assignment:

- **Keyboard interrupts** (Assignment 3) for reading input
- **PIT timing** (Assignment 4) for the `uptime` command and sleep functions
- **Memory management** (Assignment 4) for the `sysinfo` and `meminfo` commands
- **VGA output** (Assignment 2) for rendering text
- **PC speaker** (Assignment 5) for the `beep` command

The shell runs in a simple loop: print a prompt, read a line, parse the command, execute it. String comparison is done with a custom `str_equal()` function since we do not have access to the standard C library.

### Available Commands

**help** — Prints a list of all available commands.

**clear** — Clears the VGA text buffer and resets the cursor.

**sysinfo** — Displays a detailed overview of the system state. This includes uptime (calculated from PIT ticks), heap memory usage, paging status, PIT frequency, and VGA buffer address. The uptime calculation converts raw tick counts to hours, minutes, and seconds:

```c
uint32_t ticks   = get_current_tick();
uint32_t seconds = ticks / 1000;
uint32_t minutes = seconds / 60;
uint32_t hours   = minutes / 60;
```

This works because our PIT is configured at 1000 Hz, meaning each tick is exactly one millisecond.

**meminfo** — Calls the existing `print_memory_layout()` function from Assignment 4 to show heap start/end, memory used, and memory free.

**uptime** — Shows how long the system has been running since boot.

**echo \<text\>** — Prints everything after "echo " back to the screen. Uses `str_starts_with()` to detect the command prefix and prints from offset 5 onward.

**beep** — Plays a short 1 kHz tone through the PC speaker using PIT channel 2, the same mechanism from Assignment 5. The speaker is enabled for 200 ms and then turned off.

**shutdown** — Powers off the system with a countdown timer. It sends the ACPI shutdown command to the QEMU port (0x604) after displaying a "3... 2... 1..." countdown using `sleep_interrupt()`.

**exit** — Returns to the main menu.


## 6.3 Typing Speed Test

The typing speed test is an interactive feature that measures how fast and accurately the user can type. It presents words one at a time and records the time taken using our PIT timer.

### How it works

The test runs for 5 rounds. Each round picks a random word from a built-in word bank of 20 OS-related terms (like "kernel", "interrupt", "paging", "scheduler"). The randomness comes from a simple XOR-shift pseudo-random number generator seeded from the PIT tick counter:

```c
static uint32_t rng_next(void)
{
    rng_state ^= rng_state << 13;
    rng_state ^= rng_state >> 17;
    rng_state ^= rng_state << 5;
    return rng_state;
}
```

We seed this with `get_current_tick()`, which gives us a different starting value each time because the user takes a slightly different amount of time to reach this point after booting.

### Real-time colour feedback

As the user types, each character in the target word changes colour in real time by writing directly to the VGA text buffer:

- **Green (0x0A)** — the typed character matches the target
- **Red (0x0C)** — the typed character is wrong
- **Grey (0x07)** — not yet typed

This is done by the `draw_word()` function, which compares the typed string against the target character by character and writes the appropriate colour attribute byte directly to `0xB8000`.

### Timing and WPM calculation

The timer does not start when the word appears — it starts when the user presses the first key. This means thinking time is not penalised, only actual typing speed. The elapsed time is measured in PIT ticks (milliseconds):

```c
uint32_t start_tick = get_current_tick();
/* ... user types ... */
uint32_t end_tick = get_current_tick();
uint32_t elapsed_ms = end_tick - start_tick;
```

Words per minute is calculated using the standard formula: WPM = (characters / 5) / (minutes). Since we work in milliseconds, this becomes:

```
WPM = (word_length * 60000) / (5 * elapsed_ms)
```

After all 5 rounds, the test shows a summary with total time and average time per word.


## 6.4 Starfield Simulator

The starfield simulator creates a 3D flying-through-space effect entirely in VGA text mode. Stars appear to fly toward the viewer from the centre of the screen, creating the illusion of moving through a field of stars.

### 3D to 2D projection

Each star has a three-dimensional position (x, y, z). The x and y coordinates range from -1000 to 1000, and z represents depth (distance from the viewer), ranging from 1 to 64. Every frame, we decrease each star's z value by 1, making it appear closer. The 3D position is projected onto the 2D screen using perspective division:

```c
int32_t screen_x = center_x + (star.x * 2) / star.z;
int32_t screen_y = center_y + (star.y) / star.z;
```

Dividing by z is what creates the perspective effect: stars far away (large z) barely move on screen, while stars close to the viewer (small z) move rapidly toward the edges. The multiplication by 2 on the x-axis compensates for VGA text characters being roughly twice as tall as they are wide.

### Depth-based appearance

Stars change both their character and colour based on how far away they are. Distant stars appear as dim dots, while close stars are bright and large:

| Depth        | Character | Colour        |
|-------------|-----------|---------------|
| Very close  | @         | Bright white  |
| Close       | O         | Bright white  |
| Medium      | o         | Light grey    |
| Far         | *         | Dark grey     |
| Very far    | .         | Dark grey     |

### Efficient rendering

Rather than clearing the entire screen every frame (which would cause visible flicker), we track each star's previous screen position and erase only that cell before drawing the star at its new position. This technique eliminates flicker while keeping the frame rate smooth at approximately 20 FPS (50 ms per frame using `sleep_interrupt()`).

When a star flies past the viewer (z reaches 0) or moves off the edges of the screen, it is "respawned" at a random position far away, creating the impression of an endless field.


## 6.5 Menu System

The menu is the entry point to our operating system. After the kernel initialises all hardware components (GDT, IDT, PIC, PIT, memory, paging), it launches the menu, which presents the user with five options:

1. **Interactive Shell** — launches the command-line shell
2. **Typing Speed Test** — starts the typing challenge
3. **Starfield Simulator** — runs the visual effect
4. **Play Song** — plays the Super Mario theme from Assignment 5
5. **Shutdown** — powers off the system

The menu reads a single keypress (1–5) using `getchar()` and dispatches to the appropriate function. When any feature finishes (the user types "exit" in the shell, completes the typing test, or presses ESC in the starfield), control returns to the menu. This loop runs indefinitely until the user chooses to shut down.

The song player was modified so that it plays the song once and returns, rather than looping forever as it did in Assignment 5. This way the user can listen to the song and then go back to the menu to try other features.


## 6.6 Conclusion and Reflection

Building these features for Assignment 6 was where the OS finally started feeling like a real system rather than a collection of separate exercises. The most satisfying part was seeing how naturally everything connected — the shell command `sysinfo` pulls data from the memory manager we built in Assignment 4, the typing test relies on the precise millisecond timing from the PIT we configured in Assignment 4, and the starfield would not be possible without the keyboard interrupts from Assignment 3 (to detect ESC and exit).

The input system turned out to be the most important piece. Without a proper way to block and wait for keyboard input, none of the interactive features would have been possible. The ring buffer pattern — with the IRQ handler writing to one end and user code reading from the other — is the same fundamental design used in real operating systems for handling I/O.

The biggest challenge we encountered was a subtle bug with the `-fPIE` compiler flag. Our kernel is compiled as a position-independent executable, which means global variable access goes through the GOT (Global Offset Table). In a bare-metal environment without a dynamic linker, this caused the interrupt handler and normal code to resolve the same ring buffer variables to different physical addresses. We spent time debugging this by writing diagnostic values directly to the VGA buffer from both the interrupt handler and `getchar()`, which confirmed that the interrupt handler was writing characters successfully but `getchar()` never saw them. The fix was to place the shared buffer at a fixed physical address (`0x80000`), similar to how VGA memory is accessed at `0xB8000`. This experience taught us why real OS kernels are careful about how they handle shared data between interrupt context and normal execution.

Another challenge was avoiding screen flicker in the starfield. Initially we cleared the entire screen each frame, which caused noticeable blinking. Switching to selective erasure (tracking each star's previous position and only clearing that specific cell) eliminated the problem entirely.

If we had more time, we would have liked to add a simple file system so that typing test scores could be saved between sessions, and perhaps implement basic multitasking so the uptime clock could update in a corner of the screen while the shell runs.
