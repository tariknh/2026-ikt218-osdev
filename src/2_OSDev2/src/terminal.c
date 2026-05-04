#include <libc/stddef.h>
#include <libc/stdint.h>

#include "terminal.h"

static volatile uint16_t* VGA_BUFFER = (uint16_t*) 0xB8000; // VGA text mode buffer address
static uint8_t VGA_WIDTH = 80;  // VGA text mode width in characters
static uint8_t VGA_HEIGHT = 25; // VGA text mode height in characters
static uint8_t VGA_COLOR = 0x0F; // White on black

static size_t terminal_row = 0; // Current row in the terminal
static size_t terminal_column = 0;  // Current column in the terminal

static void terminal_scroll(void);

static inline uint16_t vga_entry(unsigned char c, uint8_t color) {  // Combine character and color into a single 16-bit value
    return (uint16_t) c | (uint16_t) color << 8;    // Character is in the lower byte, color is in the upper byte
}

static void terminal_putchar(char c) {  // Write a single character to the terminal, handling newlines and scrolling
    if (c == '\n') {    // Handle newline: move to the beginning of the next line
        terminal_column = 0;
        terminal_row++;

        if (terminal_row >= VGA_HEIGHT) {
            terminal_scroll();  // Scroll the terminal if we've reached the bottom
        }
        return;
    }

    if (c == '\b') {    // Handle backspace: move back one character and clear it
        if (terminal_column > 0) {
            terminal_column--;
        }

        VGA_BUFFER[terminal_row * VGA_WIDTH + terminal_column] = vga_entry(' ', VGA_COLOR);
        return;
    }

    VGA_BUFFER[terminal_row * VGA_WIDTH + terminal_column] = vga_entry(c, VGA_COLOR);

    terminal_column++;
    if (terminal_column >= VGA_WIDTH) {   // If we've reached the end of the line, move to the next line
        terminal_column = 0;
        terminal_row++;

        if (terminal_row >= VGA_HEIGHT) {   // Scroll the terminal if we've reached the bottom
            terminal_scroll();
        }
    }
}

void terminal_init(void) {  // Initialize terminal state and clear the screen
    terminal_row = 0;
    terminal_column = 0;
    VGA_COLOR = 0x0F; // White on black
    terminal_clear();
}

void terminal_clear(void) {
    terminal_row = 0;
    terminal_column = 0;

    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            VGA_BUFFER[y * VGA_WIDTH + x] = vga_entry(' ', VGA_COLOR);
        }
    }
}

static void terminal_scroll(void) {   // Scroll the terminal up by one line by copying each line to the one above it
    for (size_t y = 1; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            VGA_BUFFER[(y - 1) * VGA_WIDTH + x] = VGA_BUFFER[y * VGA_WIDTH + x];
        }
    }

    for (size_t x = 0; x < VGA_WIDTH; x++) {    // Clear the last line after scrolling
        VGA_BUFFER[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = vga_entry(' ', VGA_COLOR);
    }

    terminal_row = VGA_HEIGHT - 1;
    terminal_column = 0;
}

void terminal_write(const char* s) {    // Write a null-terminated string to the terminal
    for (size_t i = 0; s[i] != '\0'; i++) {
        terminal_putchar(s[i]);
    }
}

void terminal_write_dec(uint32_t n) {    // Write a 32-bit unsigned integer in decimal format
    if (n == 0) {   // Special case for zero
        terminal_putchar('0');
        return;
    }

    char buffer[16]; // Buffer to hold the decimal representation (enough for 32-bit integers)
    int i = 0;

    while (n) {   // Convert the number to decimal by repeatedly dividing by 10
        buffer[i++] = '0' + (n % 10); // Get the last digit and convert it to a character
        n /= 10;    // Remove the last digit
    }

    while (i > 0) {   // Write the digits in reverse order
        terminal_putchar(buffer[--i]);
    }
}

void terminal_write_hex(uint32_t n) {
    terminal_write("0x");

    char hex_digits[] = "0123456789ABCDEF"; 
    char buffer[8]; // Buffer to hold the hexadecimal representation (enough for 32-bit integers)
    int i = 0;

    if (n == 0) {   // Special case for zero
        terminal_putchar('0');
        return;
    }

    while (n > 0) {
        buffer[i++] = hex_digits[n & 0xF]; // Get the last 4 bits and convert to a hex character
        n >>= 4;    // Remove the last 4 bits
    }

    while (i > 0) {   // Write the hex digits in reverse order
        terminal_putchar(buffer[--i]);
    }
}
