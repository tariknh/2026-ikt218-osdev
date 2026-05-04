#include <keyboard.h>
#include <io.h>
#include <terminal.h>
#include <libc/stdint.h>

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_BUFFER_SIZE 256

/*
 * Simple keyboard buffer.
 */
static char keyboard_buffer[KEYBOARD_BUFFER_SIZE];
static uint32_t keyboard_buffer_index = 0;
static volatile char keyboard_last_key = 0;
static bool keyboard_echo_enabled = true;

/*
 * Handles only simple key presses without Shift, Ctrl, Alt, Caps Lock, or extended scancodes.
 */
static const char scancode_to_ascii[128] = {
    0,  27, '1', '2', '3', '4', '5', '6',
    '7', '8', '9', '0', '-', '=', '\b', '\t',
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i',
    'o', 'p', '[', ']', '\n', 0,  'a', 's',
    'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',
    '\'', '`', 0,  '\\', 'z', 'x', 'c', 'v',
    'b', 'n', 'm', ',', '.', '/', 0,  '*',
    0,  ' ', 0,  0,   0,   0,   0,   0,
    0,   0,  0,  0,   0,   0,   0,   0,
    0,   0,  0,  0,   0,   0,   0,   '7',
    '8', '9', '-', '4', '5', '6', '+', '1',
    '2', '3', '0', '.', 0,   0,   0,   0,
    0
};

static void keyboard_store_char(char character)
{
    if (keyboard_buffer_index < KEYBOARD_BUFFER_SIZE - 1) {
        keyboard_buffer[keyboard_buffer_index] = character;
        keyboard_buffer_index++;
        keyboard_buffer[keyboard_buffer_index] = '\0';
    }
}

char keyboard_get_last_key(void)
{
    char key = keyboard_last_key;
    keyboard_last_key = 0;
    return key;
}

void keyboard_set_echo(bool enabled)
{
    keyboard_echo_enabled = enabled;
}

void keyboard_handle_irq(void)
{
    uint8_t scancode = inb(KEYBOARD_DATA_PORT);

    /*
     * Ignores key releases and only handle key presses.
     */
    if (scancode & 0x80) {
        return;
    }

    char character = scancode_to_ascii[scancode];

    /*
     * Ignores unsupported keys such as Shift, Ctrl, Alt, and function keys.
     */
    if (character == 0) {
        return;
    }

    keyboard_store_char(character);
    keyboard_last_key = character;

    if (keyboard_echo_enabled) {
        terminal_putchar(character);
    }
}
