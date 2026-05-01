#include "io.h"
#include "terminal.h"
#include "isr.h"

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

void keyboard_handler(registers_t *r) {
    (void)r;
    
    uint8_t scancode = inb(0x60);
    
    if (scancode < 128) {
        char ascii = scancode_to_ascii[scancode];
        if (ascii != 0) {
            terminal_putchar(ascii);
        }
    }
}

// Function to get the buffer contents 
char* get_keyboard_buffer() {
    return keyboard_buffer;
}