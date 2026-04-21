# Assignment Four — Memory Management and PIT

## Overview

This assignment adds two foundational subsystems to the kernel:

1. **Memory Manager**: a bump allocator that provides `malloc` and `free`, placed immediately after the kernel image in physical memory, with identity-mapped paging enabled on top.
2. **Programmable Interval Timer (PIT)**: a driver for the 8253/8254 PIT chip that fires IRQ0 at 1000 Hz and exposes two sleep primitives — one using busy-waiting and one using interrupt-driven halting.

Together these give the kernel a working heap and a reliable time base, both of which are required for the music player goal: note timing depends on the PIT, and future audio data structures will use the heap.

---

## Files Changed or Created

| File | Role |
|------|------|
| `include/memory.h` | Public API for the memory manager |
| `src/memory.c` | Bump allocator, paging setup, memory layout printer |
| `include/pit.h` | PIT port/frequency macros and public API |
| `src/pit.c` | PIT initialisation, IRQ0 handler, `sleep_busy`, `sleep_interrupt` |
| `src/kernel.c` | Added `extern uint32_t end`, wired all init calls, added test loop |
| `CMakeLists.txt` | Added `src/memory.c` and `src/pit.c` to the kernel build target |

---

## Part 1 — Memory Manager

### Why a memory manager

The kernel has no operating system beneath it, so there is no `malloc` provided by a C library. Any data structure that needs to be allocated at runtime requires a kernel-owned heap. The assignment also requires paging to be enabled, which is a prerequisite for more advanced memory management later.

### The `end` symbol

The linker script (`src/arch/i386/linker.ld`) defines a symbol at the very end of the kernel image:

```
end = .; _end = .; __end = .;
```

This symbol is declared in `kernel.c` as:

```c
extern uint32_t end;
```

Its *address* (not its value) is the first byte of free physical memory after the kernel's `.bss` section. Passing `&end` to `init_kernel_memory` gives the allocator its starting point.

### Bump allocator

A bump allocator is the simplest possible heap: a single pointer starts at the base of the free region and is advanced forward on every allocation. There is no bookkeeping, no free list, and no fragmentation handling.

```
heap_start                   heap_current
    |                             |
    [  malloc(12345)  ][malloc(54321)][malloc(13331)] ...
```

`init_kernel_memory(kernel_end)` page-aligns the starting address and stores it. `malloc(size)` returns the current pointer and advances it by `size` rounded up to the next 4-byte boundary. `free` is intentionally a no-op — the bump allocator has no metadata to track individual allocations, and freeing individual blocks is not needed for this stage.

**4-byte alignment** is enforced on every allocation because unaligned 32-bit accesses fault on some hardware and are slower on all of it. The alignment is done with a simple bitmask:

```c
(addr + 3) & ~3u
```

### Paging

#### Why paging is enabled here

The x86 CPU in 32-bit protected mode can run with paging disabled (the Limine bootloader leaves it off). Enabling paging is required by the assignment and is a prerequisite for virtual memory later. It also protects the kernel from accidental writes to unmapped regions — any access outside a mapped page causes a #PF rather than silently corrupting memory.

#### Identity mapping

An identity map means virtual address == physical address for every mapped page. This is the simplest possible page table layout: the CPU can switch paging on without the kernel needing to update any pointer, because nothing in the address space moves.

The first 4 MB (1024 pages × 4 KB) is identity-mapped. This covers:

- The kernel image (loaded at 1 MB = `0x100000`)
- The VGA framebuffer (`0xB8000`)
- The heap (starts just above the kernel image)
- The stack (also near 1 MB)
- The page directory and page table themselves (in `.bss`, just above the kernel)

#### Data structures

Two 4 KB-aligned static arrays live in `.bss`:

```c
static uint32_t page_directory[1024] __attribute__((aligned(4096)));
static uint32_t page_table_0[1024]   __attribute__((aligned(4096)));
```

`__attribute__((aligned(4096)))` instructs the compiler and linker to place each array on a 4 KB boundary, which is a hardware requirement for CR3 and page directory entries.

Each entry in `page_table_0` maps one 4 KB page:

```
entry[i] = (i * 0x1000) | 0x3
```

Bits 1:0 set to `0x3` means **Present** (bit 0) and **Read/Write** (bit 1). The upper 20 bits are the physical page frame number.

`page_directory[0]` points at `page_table_0` with the same flags. All other directory entries are left at 0 (not present), so any access above 4 MB page-faults.

#### Enabling paging

```c
asm volatile("mov %0, %%cr3" : : "r"(page_directory) : "memory");
uint32_t cr0;
asm volatile("mov %%cr0, %0" : "=r"(cr0));
cr0 |= 0x80000000u;
asm volatile("mov %0, %%cr0" : : "r"(cr0) : "memory");
```

1. **CR3** is loaded with the physical address of the page directory. The CPU uses CR3 to locate the top of the two-level page table hierarchy.
2. **CR0 bit 31 (PG)** is set to activate paging. After this instruction, every memory access goes through the page table.

The `"memory"` clobber on both CR3 and CR0 writes prevents the compiler from reordering memory accesses across the paging switch.

### `print_memory_layout`

Prints four values to the terminal so the boot output can be verified visually:

- **Kernel start**: hardcoded `0x100000` (the Multiboot2 load address from the linker script)
- **Kernel end**: `heap_start`, i.e. the first free byte after the kernel image
- **Heap start**: same as kernel end
- **Heap current**: advances as `malloc` is called, showing how much heap has been consumed

---

## Part 2 — Programmable Interval Timer

### Why the PIT

The 8253/8254 PIT is the simplest hardware timer available on x86. It fires IRQ0 at a programmable rate and is the standard way to implement millisecond-granularity delays in a bare-metal kernel. For the music player, the PIT will control note durations — each note is held for a fixed number of milliseconds before the next one plays.

### Hardware background

The PIT has a fixed input clock of **1 193 180 Hz** (derived from the original IBM PC crystal). It counts down from a 16-bit divisor and fires IRQ0 each time it reaches zero, then reloads and repeats. The frequency of IRQ0 is therefore:

```
f = 1 193 180 / divisor
```

Setting `divisor = 1193` gives `f ≈ 1000 Hz`, meaning IRQ0 fires once per millisecond. This makes `tick_count` directly usable as a millisecond counter with no conversion needed (`TICKS_PER_MS = 1`).

### Initialisation

`init_pit` programs PIT channel 0 using the command port (`0x43`):

```
Command byte 0x36 = 0b 00 11 011 0
                         ^^         channel 0
                           ^^       access mode: lobyte then hibyte
                             ^^^    mode 3: square wave generator
                                ^   binary counting
```

The 16-bit divisor is then written to the channel 0 data port (`0x40`) in two bytes, low byte first. After programming the hardware, `irq_install_handler(0, pit_handler)` registers the tick handler in the existing IRQ dispatch table from assignment three, and `pic_unmask_irq(0)` enables IRQ0 on the PIC.

### IRQ0 handler

```c
static volatile uint32_t tick_count = 0;

static void pit_handler(void) {
    tick_count++;
}
```

`volatile` is required because `tick_count` is written inside an interrupt handler and read in the main execution context. Without `volatile`, the compiler is free to cache the value in a register and never re-read it, causing sleep loops to spin forever.

### `sleep_busy`

Implements a **busy-wait**: the CPU loops continuously, re-reading `tick_count` on every iteration. This wastes all available CPU cycles for the duration of the sleep but requires no hardware cooperation beyond the PIT firing IRQ0.

```
elapsed = 0
while elapsed < ticks_to_wait:
    while tick_count == start + elapsed:
        (spin)
    elapsed++
```

The inner loop waits for the tick count to advance by exactly one, then the outer loop counts how many ticks have passed. This avoids the edge case where the CPU reads `tick_count` between two ticks and skips a count.

### `sleep_interrupt`

Implements an **interrupt-driven halt**: the CPU executes `hlt` to stop issuing instructions, wakes only when an interrupt fires (the PIT IRQ), checks whether enough time has passed, and halts again if not.

```c
asm volatile("sti; hlt");
```

`sti` must immediately precede `hlt` because an interrupt gate clears IF on entry, so IF may be clear when control returns from the IRQ handler. The `sti; hlt` pair is atomic in the sense that the CPU guarantees at least one interrupt will be served before the next `hlt` goes through — avoiding the race where IF is set but an interrupt arrives before `hlt` executes.

This mode uses negligible CPU while sleeping, making it the correct choice for all delays in the music player.

---

## Boot Sequence After This Assignment

```
_start (multiboot2.asm)
  └─ main (kernel.c)
       ├─ gdt_init()               — install kernel GDT
       ├─ terminal_init()          — set up VGA output
       ├─ idt_init()               — load IDT, register ISR and IRQ stubs
       ├─ [ISR tests]              — fire int $0x0, $0x3, $0x6
       ├─ init_kernel_memory(&end) — set heap base just above kernel image
       ├─ init_paging()            — identity-map 0–4 MB, enable CR0.PG
       ├─ print_memory_layout()    — print kernel/heap addresses
       ├─ malloc() × 3             — verify bump allocator
       ├─ pic_init()               — remap PIC, mask all IRQs
       ├─ keyboard_init()          — register IRQ1 handler, unmask IRQ1
       ├─ init_pit()               — program PIT at 1000 Hz, unmask IRQ0
       ├─ sti                      — enable hardware interrupts
       └─ for(;;)                  — alternating sleep_busy / sleep_interrupt loop
```

---

## Key References

- Intel SDM Vol. 3A §4 — Paging
- OSDev Wiki — [Paging](https://wiki.osdev.org/Paging)
- OSDev Wiki — [Programmable Interval Timer](https://wiki.osdev.org/Programmable_Interval_Timer)
- OSDev Wiki — [Memory Map (x86)](https://wiki.osdev.org/Memory_Map_(x86))
