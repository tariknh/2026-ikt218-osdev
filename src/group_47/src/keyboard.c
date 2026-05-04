#include "io.h"
#include "terminal.h"
#include "isr.h"
#include "song.h"
#include "snake.h"

//This file contains the translation of keyboard scancodes to ASCII characters and the implementation of the keyboard interrupt handler.
// Scancode to ASCII lookup table (US keyboard layout)
static const char scancode_to_ascii[] = {
    0,   0,   '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=',  // 0x00-0x0D
    0,   0,   'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']',  // 0x0E-0x1B
    0,   0,   'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',  // 0x1C-0x29
    0,   '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,   '*',  // 0x2A-0x37
    0,   ' ', 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,     // 0x38-0x45
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0      // 0x46-0x53
};

// Keyboard buffer
char keyboard_buffer[256];
int buffer_index = 0;
static char last_ascii_key = 0;
static int ascii_key_available = 0;

void keyboard_handler(registers_t *r) {
    (void)r;
    
    uint8_t scancode = inb(0x60);
    static uint8_t extended = 0;

    if (scancode == 0xE0) {
        extended = 1;
        return;
    }

    if (scancode & 0x80) {
        extended = 0;
        return; 
    }


    // Handle arrow keys for snake game
    if (extended){
        switch (scancode) {
        case 0x48: snake_set_direction(DIR_UP); break;
        case 0x4D: snake_set_direction(DIR_RIGHT); break;
        case 0x50: snake_set_direction(DIR_DOWN); break;
        case 0x4B: snake_set_direction(DIR_LEFT); break;
        }
        extended = 0;
        return;
    }
    
    if (scancode < 128) {
        char ascii = scancode_to_ascii[scancode];

        if (ascii != 0) {
            last_ascii_key = ascii;
            ascii_key_available = 1;

            terminal_putchar(ascii);
        }
    }
}

// Function to check if a key has been pressed
int is_key_pressed(void) {
    return ascii_key_available;
}

// Function to get the last pressed key as an ASCII character
char get_ascii_key(void) {
    ascii_key_available = 0;
    return last_ascii_key;
}

// Function to clear the key pressed state 
void clear_key_pressed(void) {
    ascii_key_available = 0;
    last_ascii_key = 0;
}

// Function to get the buffer contents 
char* get_keyboard_buffer() {
    return keyboard_buffer;
}



