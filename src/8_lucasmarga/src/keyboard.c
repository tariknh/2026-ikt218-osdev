#include "keyboard.h"
#include "terminal.h"
#include "io.h"
#include "libc/stdint.h"

static volatile char last_key = 0;
static volatile uint8_t key_ready = 0;
static uint8_t echo_enabled = 1;

// Converts scancodes to ASCII characters
static const char keyboard_map[128] = {
    0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0,
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\',
    'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ',
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0
};

void keyboard_handle_input(void) {
    uint8_t scancode = inb(0x60);

    // Ignore key releases
    if (scancode & 0x80) {
        return;
    }

    if (scancode < 128) {
        char c = keyboard_map[scancode];

        if (c != 0) {
            last_key = c;
            key_ready = 1;

            if (echo_enabled) {
                terminal_put_char(c);
            }
        }
    }
}

char keyboard_get_key(void) {
    if (!key_ready) {
        return 0;
    }

    char c = last_key;
    key_ready = 0;
    return c;
}

char keyboard_wait_key(void) {
    char c = 0;

    while ((c = keyboard_get_key()) == 0) {
        __asm__ __volatile__("sti; hlt");
    }

    return c;
}

void keyboard_clear_buffer(void) {
    last_key = 0;
    key_ready = 0;
}

void keyboard_set_echo(uint8_t enabled) {
    echo_enabled = enabled;
}