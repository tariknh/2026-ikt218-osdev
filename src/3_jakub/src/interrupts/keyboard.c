#include "interrupts/keyboard.h"
#include "interrupts/isr.h"
#include "kernel/cli.h"
#include "common.h"
#include "apps/raycaster/raycaster.h"
#include "libc/stdio.h"

// Scancode to ASCII lookup table (US QWERTY layout).
// Index = PS/2 scancode (set 1), value = ASCII character.
// 0 means the key has no printable ASCII representation (control keys, arrows, etc.).
const char kbdUS[128] = {
    // 0x00: Unknown / 0x01: Escape / 0x02-0x0D: Top row digits and symbols
    0,
    27,
    '1',
    '2',
    '3',
    '4',
    '5',
    '6',

    // 0x08-0x0B: Remaining top row / 0x0E: Backspace / 0x0F: Tab
    '7',
    '8',
    '9',
    '0',
    '-',
    '=',
    '\b',
    '\t',

    // 0x10-0x17: Top row of letter keys (QWERTY...)
    'q',
    'w',
    'e',
    'r',
    't',
    'y',
    'u',
    'i',

    // 0x18-0x1C: (...UIOP[]) / 0x1C: Enter / 0x1D: Left Ctrl (not printable)
    'o',
    'p',
    '[',
    ']',
    '\n',
    0,
    'a',
    's',

    // 0x20-0x27: Home row letters (ASDFGHJKL;)
    'd',
    'f',
    'g',
    'h',
    'j',
    'k',
    'l',
    ';',

    // 0x28: Quote / 0x29: Grave accent / 0x2A: Left Shift / 0x2B: Backslash
    // 0x2C-0x2F: Bottom row starts (ZXCV)
    '\'',
    '`',
    0,
    '\\',
    'z',
    'x',
    'c',
    'v',

    // 0x30-0x35: Bottom row continues (BNM,.) / 0x36: Right Shift / 0x37: Keypad *
    'b',
    'n',
    'm',
    ',',
    '.',
    '/',
    0,
    '*',

    // 0x38: Left Alt / 0x39: Space / 0x3A: Caps Lock / 0x3B-0x3F: F1-F5
    0,
    ' ',
    0,
    0,
    0,
    0,
    0,
    0,

    // 0x40-0x47: F6-F10 / 0x45: Num Lock / 0x46: Scroll Lock / 0x47: Keypad 7 (Home)
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,

    // 0x48: Keypad 8 (Up) / 0x49: Keypad 9 (Page Up) / 0x4A: Keypad -
    // 0x4B: Keypad 4 (Left) / 0x4C: Keypad 5 / 0x4D: Keypad 6 (Right) / 0x4E: Keypad +
    // 0x4F: Keypad 1 (End)
    0,
    '-',
    0,
    0,
    0,
    '+',
    0,
    0,

    // 0x50: Keypad 2 (Down) / 0x51: Keypad 3 (Page Down)
    // 0x52: Keypad 0 (Insert) / 0x53: Keypad . (Delete) / 0x54-0x56: Undefined
    // 0x57: F11 / 0x58: F12
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,

    // 0x58-0x7F: All remaining scancodes are undefined in set 1
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
};

// Scancode to ASCII lookup table for shifted keys (US QWERTY layout).
// Used when Left or Right Shift is held. Mirrors kbdUS in structure,
// but digits become symbols, letters become uppercase, and some punctuation changes.
const char kbdUS_shift[128] = {
    // 0x00: Unknown / 0x01: Escape / 0x02-0x0D: Top row shifted (digits -> symbols)
    0,
    27,
    '!',
    '@',
    '#',
    '$',
    '%',
    '^',

    // 0x08-0x0B: Remaining shifted symbols / 0x0E: Backspace / 0x0F: Tab (unchanged)
    '&',
    '*',
    '(',
    ')',
    '_',
    '+',
    '\b',
    '\t',

    // 0x10-0x17: Top row letters, uppercased (QWERTY...)
    'Q',
    'W',
    'E',
    'R',
    'T',
    'Y',
    'U',
    'I',

    // 0x18-0x1B: (...OPP{}) / 0x1C: Enter / 0x1D: Left Ctrl (not printable)
    'O',
    'P',
    '{',
    '}',
    '\n',
    0,
    'A',
    'S',

    // 0x20-0x27: Home row letters, uppercased (ASDFGHJKL:)
    'D',
    'F',
    'G',
    'H',
    'J',
    'K',
    'L',
    ':',

    // 0x28: Double quote / 0x29: Tilde / 0x2A: Left Shift / 0x2B: Pipe
    // 0x2C-0x2F: Bottom row starts, uppercased (ZXCV)
    '"',
    '~',
    0,
    '|',
    'Z',
    'X',
    'C',
    'V',

    // 0x30-0x35: Bottom row continues (BNMC<>?) / 0x36: Right Shift / 0x37: Keypad *
    'B',
    'N',
    'M',
    '<',
    '>',
    '?',
    0,
    '*',

    // 0x38: Left Alt / 0x39: Space (unchanged) / 0x3A: Caps Lock / 0x3B-0x3F: F1-F5
    0,
    ' ',
    0,
    0,
    0,
    0,
    0,
    0,

    // 0x40-0x7F: F6 and beyond — shift has no effect on non-printable keys
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
};

#define KBD_BUFFER_SIZE 256
char keyboard_buffer[KBD_BUFFER_SIZE];
int buffer_index = 0;
int shift_pressed = 0;
static int extended_scancode = 0;
static volatile char last_key_pressed = 0;

static void handle_extended_scancode(uint8_t scancode)
{
    uint8_t released = scancode & 0x80;
    uint8_t code = scancode & 0x7F;

    if (released)
    {
        return;
    }

    if (raycaster_input_is_active())
    {
        return;
    }

    if (code == 0x49)
    {
        terminal_scroll_page_up();
        return;
    }

    if (code == 0x51)
    {
        terminal_scroll_page_down();
        return;
    }

    if (code == 0x48)
    {
        terminal_scroll_line_up();
        return;
    }

    if (code == 0x50)
    {
        terminal_scroll_line_down();
        return;
    }

    if (code == 0x47)
    {
        terminal_scroll_to_top();
        return;
    }

    if (code == 0x4F)
    {
        terminal_scroll_to_bottom();
    }
}

static void keyboard_callback(registers_t *regs)
{
    (void)regs;
    // The PIC leaves us the scancode in port 0x60
    uint8_t scancode = inb(0x60);

    if (raycaster_input_is_active())
    {
        raycaster_input_submit_scancode(scancode);
    }

    if (scancode == 0xE0)
    {
        extended_scancode = 1;
        return;
    }

    if (extended_scancode)
    {
        extended_scancode = 0;
        handle_extended_scancode(scancode);
        return;
    }

    // Top bit set means key released
    if (scancode & 0x80)
    {
        // Key release
        uint8_t released_key = scancode & ~0x80;
        if (released_key == 0x2A || released_key == 0x36)
        { // Left shift (42) or Right shift (54)
            shift_pressed = 0;
        }
    }
    else
    {
        // Key press
        if (scancode == 0x2A || scancode == 0x36)
        {
            shift_pressed = 1;
        }
        else
        {
            char ascii = shift_pressed ? kbdUS_shift[scancode] : kbdUS[scancode];

            // ESC key - always handle specially
            if (scancode == 0x01)
            {
                ascii = 27; // ESC code
                last_key_pressed = ascii;
                raycaster_input_request_exit();
                if (!raycaster_input_is_active())
                {
                    cli_handle_escape();
                }
                return;
            }

            // Always capture the key for games
            last_key_pressed = ascii;

            // In game mode, only capture keys - don't process commands
            if (raycaster_input_is_active())
            {
                return;
            }

            // Normal command mode - process as commands
            if (ascii == '\b')
            {
                terminal_scroll_to_bottom();
                if (buffer_index > 0)
                {
                    buffer_index--;
                    keyboard_buffer[buffer_index] = '\0';
                    printf("\b \b");
                }
            }
            else if (ascii == '\n')
            {
                terminal_scroll_to_bottom();
                cli_submit_line(keyboard_buffer);
                buffer_index = 0;
                keyboard_buffer[0] = '\0';
            }
            else if (ascii != 0)
            {
                terminal_scroll_to_bottom();
                // Store in buffer
                if (buffer_index < KBD_BUFFER_SIZE - 1)
                {
                    keyboard_buffer[buffer_index++] = ascii;
                    keyboard_buffer[buffer_index] = '\0';
                }
                // Print out character
                printf("%c", ascii);
            }
        }
    }
}

void init_keyboard()
{
    keyboard_buffer[0] = '\0';
    register_interrupt_handler(IRQ1, keyboard_callback);
}

void keyboard_print_prompt(void)
{
    cli_print_prompt();
}

char keyboard_get_last_key(void)
{
    char key = last_key_pressed;
    last_key_pressed = 0; // Clear after reading
    return key;
}
