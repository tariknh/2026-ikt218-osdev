#include "terminal.h"
#include "keyboard.h"
#include <libc/stdint.h>

#define KEYBOARD_BUFFER_SIZE 256

static volatile char last_keyboard_key = '\0';

// Store raw keyboard scancodes
static uint8_t keyboard_buffer[KEYBOARD_BUFFER_SIZE];
static uint8_t keyboard_buffer_index = 0;

// Simple scancode-to-ASCII lookup table for common keys
static const char keyboard_map[128] = {
    0,   27,  '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t',
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0,
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0,
    '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',
    0,
    '*',
    0,
    ' ',
    0
};

void keyboard_handle_scancode(uint8_t scancode){
    // Ignore key-release events
    if (scancode & 0x80){
        return;
    }

    // Store scancode in the buffer if there is free space
    if (keyboard_buffer_index < KEYBOARD_BUFFER_SIZE){
        keyboard_buffer[keyboard_buffer_index] = scancode;
        keyboard_buffer_index++;
    }

    // Translate scancode to ASCII and print it
    if (scancode < 128){
        char c = keyboard_map[scancode];



        // Print only valid characters
        if (c != 0){
            last_keyboard_key = c;
            terminal_putchar(c);
        }
    }
}

// Getter function for monitor
char get_last_keyboard_key() {
    return last_keyboard_key;
}