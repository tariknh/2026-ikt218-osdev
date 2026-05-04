#include "terminal.h"

static size_t terminal_row = 0;                          // Current row position in the VGA text buffer
static size_t terminal_column = 0;                       // Current column position in the VGA text buffer
static uint8_t terminal_color = 0x07;                    // Light grey text on black background (DOS default)
static uint16_t* terminal_buffer = (uint16_t*)0xB8000;   // VGA text mode memory address

// Create and return one 16-bit VGA text entry
static uint16_t vga_entry(char c, uint8_t color) {
    return (uint16_t)c | ((uint16_t)color << 8);
}

// Clear the screen and reset cursor position
void terminal_initialize(void) {
    // Begin at the start of the screen
    terminal_row = 0;
    terminal_column = 0;
    terminal_color = 0x07;

    // Loop through 25 rows and 80 columns (standard VGA text mode)
    for (size_t y = 0; y < 25; y++) {
        for (size_t x = 0; x < 80; x++) {
            size_t index = y * 80 + x;                                  // Give each row/column pair its own index
            terminal_buffer[index] = vga_entry(' ', terminal_color);    // Replace old characters with spaces using the current VGA color
        }
    }
}

// Write one letter to the current screen position
void terminal_putchar(char c) {
    if (c == '\n'){
        terminal_column = 0;
        terminal_row++;
        return;
    }

    size_t index = terminal_row * 80 + terminal_column;                 // Find current screen position
    terminal_buffer[index] = vga_entry(c, terminal_color);              // Write the character with current color

    terminal_column++;

    if (terminal_column == 80) {
        terminal_column = 0;
        terminal_row++;
    }
}

// Write full string until null terminator
void terminal_write(const char* data) {
    size_t i = 0;

    while (data[i] != '\0') {
        terminal_putchar(data[i]);
        i++;
    }
}

// Print a 32-bit value as hexadecimal
void print_hex(uint32_t value)
{
    const char* digits = "0123456789ABCDEF";

    terminal_write("0x");

    for (int i = 28; i >= 0; i -= 4) {
        uint8_t digit = (value >> i) & 0xF;
        terminal_putchar(digits[digit]);
    }
}

// Print a 32-bit value as decimals
void print_dec(uint32_t value) {
    char buffer[10];
    int index = 0;

    if (value == 0) {
        terminal_putchar('0');
        return;
    }

    while (value > 0) {
        buffer[index] = '0' + (value % 10);
        value = value / 10;
        index++;
    }

    while (index > 0) {
        index--;
        terminal_putchar(buffer[index]);
    }
}


void terminal_clear() {
    terminal_initialize();
}