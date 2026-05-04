#include <libc/stdint.h>
#include "keyboard.h"
#include "irq.h"
#include "ports.h"
#include "kernel/irq_rush.h"

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_BUFFER_SIZE 256

static uint8_t keyboard_scancode_buffer[KEYBOARD_BUFFER_SIZE];
static char keyboard_ascii_buffer[KEYBOARD_BUFFER_SIZE];

static uint32_t keyboard_scancode_count = 0;
static uint32_t keyboard_ascii_count = 0;

static const char scancode_to_ascii[128] = {
    0,   0,  '1', '2', '3', '4', '5', '6', '7', '8',
    '9', '0', '-', '=', 0,   0,
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p',
    '[', ']', '\n', 0,
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',
    '\'', '`', 0,   '\\',
    'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',
    0,   '*', 0,   ' ',
    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0
};

static void keyboard_irq_callback(struct registers* regs) {
    uint8_t scancode;
    char ascii;

    (void)regs;

    scancode = port_byte_in(KEYBOARD_DATA_PORT);

    keyboard_scancode_buffer[keyboard_scancode_count % KEYBOARD_BUFFER_SIZE] = scancode;
    keyboard_scancode_count++;

    irq_rush_handle_scancode(scancode);

    if (scancode == 0xE0 || scancode == 0xE1) {
        return;
    }

    if (scancode & 0x80) {
        return;
    }

    if (scancode >= 128) {
        return;
    }

    ascii = scancode_to_ascii[scancode];

    if (ascii == 0) {
        return;
    }

    keyboard_ascii_buffer[keyboard_ascii_count % KEYBOARD_BUFFER_SIZE] = ascii;
    keyboard_ascii_count++;
}

void keyboard_init(void) {
    register_irq_handler(1, keyboard_irq_callback);
}
