#include "interrupts/keyboard.h"
#include "interrupts/isr.h"
#include "kernel/cli.h"
#include "arch/i386/io.h"
#include "apps/raycaster/raycaster.h"
#include "apps/song/song.h"
#include "libc/stdio.h"

// Scancode to ASCII lookup table (US QWERTY layout).
// Index = PS/2 scancode (set 1), value = ASCII character.
// 0 means the key has no printable ASCII representation (control keys, arrows, etc.).
const char kbdUS[128] = {
    // 0x00: Unknown / 0x01: Escape / 0x02-0x0D: Top row digits and symbols
    0,    27,   '1',  '2',  '3',  '4',  '5',  '6',

    // 0x08-0x0B: Remaining top row / 0x0E: Backspace / 0x0F: Tab
    '7',  '8',  '9',  '0',  '-',  '=',  '\b', '\t',

    // 0x10-0x17: Top row of letter keys (QWERTY...)
    'q',  'w',  'e',  'r',  't',  'y',  'u',  'i',

    // 0x18-0x1C: (...UIOP[]) / 0x1C: Enter / 0x1D: Left Ctrl (not printable)
    'o',  'p',  '[',  ']',  '\n', 0,    'a',  's',

    // 0x20-0x27: Home row letters (ASDFGHJKL;)
    'd',  'f',  'g',  'h',  'j',  'k',  'l',  ';',

    // 0x28: Quote / 0x29: Grave accent / 0x2A: Left Shift / 0x2B: Backslash
    // 0x2C-0x2F: Bottom row starts (ZXCV)
    '\'', '`',  0,    '\\', 'z',  'x',  'c',  'v',

    // 0x30-0x35: Bottom row continues (BNM,.) / 0x36: Right Shift / 0x37: Keypad *
    'b',  'n',  'm',  ',',  '.',  '/',  0,    '*',

    // 0x38: Left Alt / 0x39: Space / 0x3A: Caps Lock / 0x3B-0x3F: F1-F5
    0,    ' ',  0,    0,    0,    0,    0,    0,

    // 0x40-0x47: F6-F10 / 0x45: Num Lock / 0x46: Scroll Lock / 0x47: Keypad 7 (Home)
    0,    0,    0,    0,    0,    0,    0,    0,

    // 0x48: Keypad 8 (Up) / 0x49: Keypad 9 (Page Up) / 0x4A: Keypad -
    // 0x4B: Keypad 4 (Left) / 0x4C: Keypad 5 / 0x4D: Keypad 6 (Right) / 0x4E: Keypad +
    // 0x4F: Keypad 1 (End)
    0,    '-',  0,    0,    0,    '+',  0,    0,

    // 0x50: Keypad 2 (Down) / 0x51: Keypad 3 (Page Down)
    // 0x52: Keypad 0 (Insert) / 0x53: Keypad . (Delete) / 0x54-0x56: Undefined
    // 0x57: F11 / 0x58: F12
    0,    0,    0,    0,    0,    0,    0,    0,

    // 0x58-0x7F: All remaining scancodes are undefined in set 1
    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,
};

// Scancode to ASCII lookup table for shifted keys (US QWERTY layout).
// Used when Left or Right Shift is held. Mirrors kbdUS in structure,
// but digits become symbols, letters become uppercase, and some punctuation changes.
const char kbdUS_shift[128] = {
    // 0x00: Unknown / 0x01: Escape / 0x02-0x0D: Top row shifted (digits -> symbols)
    0,    27,   '!',  '@',  '#',  '$',  '%',  '^',

    // 0x08-0x0B: Remaining shifted symbols / 0x0E: Backspace / 0x0F: Tab (unchanged)
    '&',  '*',  '(',  ')',  '_',  '+',  '\b', '\t',

    // 0x10-0x17: Top row letters, uppercased (QWERTY...)
    'Q',  'W',  'E',  'R',  'T',  'Y',  'U',  'I',

    // 0x18-0x1B: (...OPP{}) / 0x1C: Enter / 0x1D: Left Ctrl (not printable)
    'O',  'P',  '{',  '}',  '\n', 0,    'A',  'S',

    // 0x20-0x27: Home row letters, uppercased (ASDFGHJKL:)
    'D',  'F',  'G',  'H',  'J',  'K',  'L',  ':',

    // 0x28: Double quote / 0x29: Tilde / 0x2A: Left Shift / 0x2B: Pipe
    // 0x2C-0x2F: Bottom row starts, uppercased (ZXCV)
    '"',  '~',  0,    '|',  'Z',  'X',  'C',  'V',

    // 0x30-0x35: Bottom row continues (BNMC<>?) / 0x36: Right Shift / 0x37: Keypad *
    'B',  'N',  'M',  '<',  '>',  '?',  0,    '*',

    // 0x38: Left Alt / 0x39: Space (unchanged) / 0x3A: Caps Lock / 0x3B-0x3F: F1-F5
    0,    ' ',  0,    0,    0,    0,    0,    0,

    // 0x40-0x7F: F6 and beyond — shift has no effect on non-printable keys
    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,
};

#define KBD_BUFFER_SIZE 256
char keyboard_buffer[KBD_BUFFER_SIZE]; // Buffer to store the current command being typed by the user
int buffer_index = 0; // Index to keep track of the current position in the keyboard buffer
int shift_pressed = 0; // Flag to track whether either Shift key is currently pressed (0 = not pressed, 1 = pressed)
static int extended_scancode = 0; // Flag to track whether the last scancode was the start of an extended sequence (0 = no, 1 = yes)
static volatile char last_key_pressed = 0; // Store the last key pressed for games to read asynchronously

// Route ESC based on active mode
static void handle_escape_key(void)
{
    if (raycaster_input_is_active()) {
        // In game mode ESC should exit game and silence audio
        raycaster_input_request_exit();
        stop_sound();
        return;
    }

    // In terminal mode ESC stops music playback
    stop_music();
}

static void handle_extended_scancode(uint8_t scancode) // Handle special keys that send extended scancodes (those that start with 0xE0)
{
    uint8_t released = scancode & 0x80; // Check if the top bit is set to determine if it's a key release (1) or key press (0)
    uint8_t code = scancode & 0x7F; // Get the actual scancode by masking out the top bit

    if (released) { // we only care about key presses for extended keys, so ignore releases
        return;
    }

    if (raycaster_input_is_active()) { // In game mode, we don't want to process extended keys as commands, so just ignore them
        return;
    }

    if (code == 0x49) { // Page Up
        terminal_scroll_page_up();
        return;
    }

    if (code == 0x51) { // Page Down
        terminal_scroll_page_down();
        return;
    }

    if (code == 0x48) { // Up Arrow
        terminal_scroll_line_up();
        return;
    }

    if (code == 0x50) { // Down Arrow
        terminal_scroll_line_down();
        return;
    }

    if (code == 0x47) { // Home
        terminal_scroll_to_top();
        return;
    }

    if (code == 0x4F) { // End
        terminal_scroll_to_bottom();
    }
}

static void keyboard_callback(registers_t *regs) { // This function is called every time a keyboard interrupt occurs (IRQ1)
    (void)regs;
    // The PIC leaves us the scancode in port 0x60
    uint8_t scancode = inb(0x60);

    if (raycaster_input_is_active()) { // If the game is active, we want to capture all keys (including extended keys) and send them to the game input handler, without processing any commands or special keys
        // Forward raw scancodes to the game input queue
        raycaster_input_submit_scancode(scancode);
    }

    if (scancode == 0xE0) { // 0xE0 indicates the start of an extended scancode sequence, so we set a flag and wait for the next scancode to determine which special key it is
        extended_scancode = 1;
        return;
    }

    if (extended_scancode) { // If the extended scancode flag is set, we know the current scancode is part of an extended sequence, so we handle it accordingly and then clear the flag
        extended_scancode = 0;
        // Handle arrows and paging keys
        handle_extended_scancode(scancode);
        return;
    }

    // Top bit set means key released
    if (scancode & 0x80) { // Check if the top bit is set to determine if it's a key release (1) or key press (0)
        // Key release
        uint8_t released_key = scancode & ~0x80;
        if (released_key == 0x2A || released_key == 0x36) { // Left shift (42) or Right shift (54)
            shift_pressed = 0;
        }

    } else {
        // Key press
        if (scancode == 0x2A || scancode == 0x36) {
            shift_pressed = 1;
        } else {
            char ascii = shift_pressed ? kbdUS_shift[scancode] : kbdUS[scancode]; // Use the appropriate lookup table based on whether Shift is pressed to convert the scancode to an ASCII character. If the scancode doesn't correspond to a printable character, this will be 0.
            
            // ESC key - always handle specially
            if (scancode == 0x01) {
                ascii = 27;  // ESC code
                last_key_pressed = ascii;
                handle_escape_key();
                return;
            }
            
            // Always capture the key for games
            last_key_pressed = ascii;

            // In game mode, only capture keys - don't process commands
            if (raycaster_input_is_active()) {
                return;
            }
            
            // Normal command mode - process as commands
            if (ascii == '\b') {
                terminal_scroll_to_bottom(); // Ensure we are at the bottom of the terminal when processing backspace, so the user can see the effect of their input
                if (buffer_index > 0) { // Only process backspace if there is something in the buffer to delete
                    buffer_index--; // Move back the buffer index to "delete" the last character
                    keyboard_buffer[buffer_index] = '\0'; // Null-terminate the buffer at the new index to effectively remove the last character
                    printf("\b \b"); // Move the cursor back, print a space to erase the character on the screen, and move back again to position the cursor correctly for the next input. This gives the visual effect of deleting the last character typed by the user.
                }
            } else if (ascii == '\n') { // When the user presses Enter, we want to submit the current buffer as a command to the CLI, and then clear the buffer for the next command. We also scroll to the bottom of the terminal to ensure the user can see the result of their command and the new prompt.
                terminal_scroll_to_bottom(); // Ensure we are at the bottom of the terminal when processing the command, so the user can see the result of their command and the new prompt
                cli_submit_line(keyboard_buffer); // Submit the current buffer as a command to the CLI for processing. The CLI will handle executing the command and printing any output or errors.
                buffer_index = 0; // Reset the buffer index to 0 to start filling the buffer from the beginning for the next command
                keyboard_buffer[0] = '\0'; // Null-terminate the buffer at the beginning to clear any previous command from the buffer. This ensures that the next command starts with an empty buffer.
            } else if (ascii != 0) { // Only process printable characters (non-zero ASCII) to add to the buffer and print on the screen. Non-printable keys (like function keys, arrow keys, etc.) will have an ASCII value of 0 in our lookup tables, so we ignore those for command input.
                terminal_scroll_to_bottom(); // Ensure we are at the bottom of the terminal when processing normal character input, so the user can see their input as they type and any resulting output from commands they execute. This also prevents the user from accidentally typing a command while scrolled up and not realizing it until they scroll back down.
                // Store in buffer
                if (buffer_index < KBD_BUFFER_SIZE - 1) {
                    keyboard_buffer[buffer_index++] = ascii; // Add the character to the buffer and increment the index for the next character. We check to ensure we don't overflow the buffer - if we reach the end, we simply stop adding characters until the user submits the command or deletes some characters.
                    keyboard_buffer[buffer_index] = '\0'; // Null-terminate the buffer at the new index to effectively add the character
                }
                // Print out character
                printf("%c", ascii);
            }
        }
    }
}

void init_keyboard() { // Initialize the keyboard by registering our keyboard callback function as the handler for IRQ1 (the keyboard interrupt), and also initialize the keyboard buffer to be empty. After this function is called, every time a key is pressed or released on the keyboard, our keyboard_callback function will be called to handle the input accordingly.
    keyboard_buffer[0] = '\0'; // Initialize the keyboard buffer to be an empty string by setting the first character to the null terminator. This ensures that when we start typing commands, we are starting with a clean buffer.
    register_interrupt_handler(IRQ1, keyboard_callback); // Register our keyboard_callback function as the handler for IRQ1, which is the hardware interrupt generated by the keyboard. This tells the interrupt handling system to call our function whenever a keyboard event occurs, allowing us to process user input from the keyboard.
}

void keyboard_print_prompt(void) // This function can be called to print a new prompt on the terminal and ensure the user is ready to type a new command. It also scrolls to the bottom of the terminal to ensure the prompt is visible and the user can see their input as they type.
{
    cli_print_prompt(); // Call the CLI function to print the command prompt (e.g., "user@os:~$ ") on the terminal, indicating to the user that they can start typing a new command.
}

char keyboard_get_last_key(void) 
{
    char key = last_key_pressed; // Store the last key pressed in a local variable to return
    last_key_pressed = 0;  // Clear after reading
    return key; // Return the last key pressed as an ASCII character. If no key has been pressed since the last call, this will return 0.
}
