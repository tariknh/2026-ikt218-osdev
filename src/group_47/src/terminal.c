#include "libc/stdint.h"
#include "libc/stddef.h"
#include "io.h"
// This file contains the implementation of the terminal output functions, which allow us to print text to the screen using the VGA text mode.
// VGA screen is 80 columns wide and 25 rows high
static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;
// Defining the variables
size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color;
uint16_t* terminal_buffer;

// Function to update the hardware cursor position based on the current terminal_row and terminal_column
void update_hardware_cursor(void) {
    uint16_t pos = terminal_row * VGA_WIDTH + terminal_column;
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

void terminal_initialize() { // Initialize the terminal screen by setting the cursor to the top-left corner, setting the default color, and clearing the screen.
    terminal_row = 0;
    terminal_column = 0;
    terminal_color = 0x0F; // White text on black background
    terminal_buffer = (uint16_t*) 0xB8000;
    
    // Clear the screen
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            terminal_buffer[index] = (uint16_t) ' ' | (uint16_t) terminal_color << 8;
        }
    }
    update_hardware_cursor();
}

// Function to write a single character after current cursor position and move the cursor forward, handling newlines and carriage returns
void terminal_putchar(char c) {
    if (c == '\n') {
        terminal_column = 0;
        terminal_row++;
    } else if (c == '\r') {
        terminal_column = 0;
    } else {
        const size_t index = terminal_row * VGA_WIDTH + terminal_column;
        terminal_buffer[index] = (uint16_t) c | (uint16_t) terminal_color << 8;
        terminal_column++;
        
        if (terminal_column >= VGA_WIDTH) {
            terminal_column = 0;
            terminal_row++;
        }
    }
    
    // Handle scrolling
    if (terminal_row >= VGA_HEIGHT) {
        // Scroll up
        for (size_t y = 1; y < VGA_HEIGHT; y++) {
            for (size_t x = 0; x < VGA_WIDTH; x++) {
                const size_t src = y * VGA_WIDTH + x;
                const size_t dst = (y - 1) * VGA_WIDTH + x;
                terminal_buffer[dst] = terminal_buffer[src];
            }
        }
        
        // Clear last line
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = (VGA_HEIGHT - 1) * VGA_WIDTH + x;
            terminal_buffer[index] = (uint16_t) ' ' | (uint16_t) terminal_color << 8;
        }
        
        terminal_row = VGA_HEIGHT - 1;
    }
    update_hardware_cursor();
}

void printf(const char* data) { // Simple implementation of printf that only supports strings (no formatting)
    for (size_t i = 0; data[i] != '\0'; i++)
        terminal_putchar(data[i]);
}

void print_int(uint32_t num) { // Function to print an integer to the terminal
    char buffer[32];
    char *ptr = buffer;
    
    if (num == 0) {
        terminal_putchar('0');
        return;
    }
    
    // Convert to string (reverse order)
    while (num > 0) {
        *ptr++ = '0' + (num % 10);
        num /= 10;
    }
    
    // Print in correct order
    while (ptr > buffer) {
        terminal_putchar(*--ptr);
    }
}

void print_hex(uint32_t num) {
    char hex_chars[] = "0123456789ABCDEF";
    char buffer[10]; // "0x" + 8 chars for 32-bit + null
    
    terminal_putchar('0');
    terminal_putchar('x');

    if (num == 0) {
        terminal_putchar('0');
        return;
    }

    // Process from right to left (8 characters for a 32-bit uint32_t)
    for (int i = 7; i >= 0; i--) {
        int nibble = (num >> (i * 4)) & 0xF;
        terminal_putchar(hex_chars[nibble]);
    }
}

// Alias for printf (for consistency)
void print(const char* str) {
    printf(str);
}

// Function to draw objects at specific coordiantes and specified color
void terminal_draw_entry(char c, uint8_t color, size_t x, size_t y) {
    if (x >= VGA_WIDTH || y >= VGA_HEIGHT) return;

    const size_t index = y * VGA_WIDTH + x;
    terminal_buffer[index] = (uint16_t)c | (uint16_t)color << 8;
}

// Function to clear the terminal screen 
void terminal_clear() {
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            terminal_buffer[index] = (uint16_t)' ' | (uint16_t)terminal_color << 8;
        }
    }

    terminal_row = 0;
    terminal_column = 0;
    update_hardware_cursor();
}