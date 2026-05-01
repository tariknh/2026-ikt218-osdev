#include <kernel/piano.h>
#include <libc/stdint.h>
#include "keyboard.h"
#include "io.h"

extern void terminal_write(const char* data);
extern void terminal_putchar(char c);

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_BUFFER_SIZE 256

static int shift_pressed = 0;
static uint8_t keyboard_buffer[KEYBOARD_BUFFER_SIZE];
static uint32_t keyboard_buffer_index = 0;

static const char scancode_to_ascii[128] = {
    0,  27, '1', '2', '3', '4', '5', '6',
    '7', '8', '9', '0', '-', '=', '\b', '\t',
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i',
    'o', 'p', '[', ']', '\n', 0, 'a', 's',
    'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',
    '\'', '`', 0, '\\', 'z', 'x', 'c', 'v',
    'b', 'n', 'm', ',', '.', '/', 0, '*',
    0, ' ', 0
};

void keyboard_init(void) {
    keyboard_buffer_index = 0;
    terminal_write("Keyboard initialized\n");
}

void keyboard_handler(void) {
    uint8_t scancode = inb(KEYBOARD_DATA_PORT);
    piano_handle_scancode(scancode);

    if (scancode == 0x2A || scancode == 0x36) {
        shift_pressed = 1;
        return;
    }

    if (scancode == 0xAA || scancode == 0xB6) {
        shift_pressed = 0;
        return;
    }

    if (scancode & 0x80) {
        return;
    }

    char ascii = scancode_to_ascii[scancode];

    if (ascii != 0) {
        if (shift_pressed && ascii >= 'a' && ascii <= 'z') {
            ascii = ascii - 32; // gjør til stor bokstav
        }

        terminal_putchar(ascii);
    }
}