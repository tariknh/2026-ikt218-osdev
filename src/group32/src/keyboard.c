#include "keyboard.h"
#include "irq.h"
#include "ports.h"

static volatile char keyboard_buffer[256]; // Keyboard input buffer
static volatile int keyboard_index = 0;// Number of keys in the buffer
static volatile int extended_scancode = 0;// Tracks extended scancodes

// Converts scancodes to ASCII
static const char scancode_to_ascii[128] = {
    0,   27, '1', '2', '3', '4', '5', '6',
    '7', '8', '9', '0', '-', '=', '\b', '\t',
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i',
    'o', 'p', '[', ']', '\n', 0,   'a', 's',
    'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',
    '\'', '`', 0,  '\\', 'z', 'x', 'c', 'v',
    'b', 'n', 'm', ',', '.', '/', 0,   '*',
    0,   ' ', 0
};

// Adds one key to the buffer
static void push_key(char c) {
    if (c != 0 && keyboard_index < 255) {
        keyboard_buffer[keyboard_index] = c;
        keyboard_index++;
        keyboard_buffer[keyboard_index] = '\0';
    }
}

// Handles keyboard IRQs
static void keyboard_handler(struct registers* regs) {
    (void)regs;

    unsigned char scancode = inb(0x60);  // Read scancode from keyboard port

    // Check for extended scancode prefix
    if (scancode == 0xE0) {
        extended_scancode = 1;
        return;
    }
    // Ignore key releases
    if (scancode & 0x80) {
        extended_scancode = 0;
        return;
    }
    // Handle extended keys
    if (extended_scancode) {
        if (scancode == 0x48) {
            push_key(KEY_ARROW_UP);
        }
        else if (scancode == 0x50) {
            push_key(KEY_ARROW_DOWN);
        }

        extended_scancode = 0;
        return;
    }
    // Convert normal scancode to ASCII
    if (scancode < 128) {
        push_key(scancode_to_ascii[scancode]);
    }
}
// Installs keyboard IRQ handler
void keyboard_install(void) {
    keyboard_index = 0;
    keyboard_buffer[0] = '\0';
    extended_scancode = 0;

    irq_install_handler(1, keyboard_handler);
}
// Waits for and returns one character
char keyboard_get_char(void) {
    while (keyboard_index == 0) {
        __asm__ volatile ("hlt"); // Wait for interrupt
    }

    return keyboard_read_char();
}
// Reads one character from the buffer
char keyboard_read_char(void) {
    if (keyboard_index == 0) {
        return 0;
    }

    char c = keyboard_buffer[0];    // Get first character
    // Shift buffer left
    for (int i = 1; i < keyboard_index; i++) {
        keyboard_buffer[i - 1] = keyboard_buffer[i];
    }
    // Update buffer size
    keyboard_index--;
    keyboard_buffer[keyboard_index] = '\0';

    return c;
}
// Checks if a key is available
int keyboard_has_key(void) {
    return keyboard_index > 0;
}