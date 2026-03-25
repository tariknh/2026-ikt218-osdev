#include "terminal.h"

// Standard VGA text mode size
static const int VGA_WIDTH = 80;
static const int VGA_HEIGHT = 25;
static uint16_t* const VGA_BUFFER = (uint16_t*) 0xB8000;

static int terminal_row;
static int terminal_column;
static uint8_t terminal_color;

static inline void outb(uint16_t port, uint8_t val) {
    asm volatile ( "outb %0, %1" : : "a"(val), "Nd"(port) );
}
// Helper to create the 2-byte VGA entry (Character + Color)
static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
    return (uint16_t) uc | (uint16_t) color << 8;
}

void vga_set_color(uint8_t index, uint8_t r, uint8_t g, uint8_t b) {
    
    outb(0x3C8, index);
    // We use I/O port 0x3C9 to send the R, G, and B values in sequence
    // Note: VGA expects 6-bit values (0-63), so we shift or divide by 4
    outb(0x3C9, r / 4);
    outb(0x3C9, g / 4);
    outb(0x3C9, b / 4);
}

void terminal_update_cursor() {
    // The cursor position is a 1d index: y * width + x
    uint16_t pos = terminal_row * VGA_WIDTH + terminal_column;

    // Send the high byte (bits 8-15)
    outb(0x3D4, 0x0E);            // We want to set register 14 (Cursor Location High)
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));

    // Send the low byte (bits 0-7)
    outb(0x3D4, 0x0F);            // We want to set register 15 (Cursor Location Low)
    outb(0x3D5, (uint8_t)(pos & 0xFF));
}

static void terminal_scroll() {
    for (int y = 0; y < VGA_HEIGHT - 1; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            const int src_index = (y + 1) * VGA_WIDTH + x;
            const int dest_index = y * VGA_WIDTH + x;
            VGA_BUFFER[dest_index] = VGA_BUFFER[src_index];
        }
    }
    // Clear the bottom-most line
    for (int x = 0; x < VGA_WIDTH; x++) {
        const int index = (VGA_HEIGHT - 1) * VGA_WIDTH + x;
        VGA_BUFFER[index] = vga_entry(' ', terminal_color);
    }
}

void terminal_putchar(char c) {
    if (c == '\n') {
        terminal_column = 0;
        if (++terminal_row == VGA_HEIGHT) {
            terminal_scroll();
            terminal_row = VGA_HEIGHT - 1;
        }
    } 
    else if (c == '\b') {
        if (terminal_column > 0) {
            terminal_column--;
            // Erase the character at the new position
            const int index = terminal_row * VGA_WIDTH + terminal_column;
            VGA_BUFFER[index] = vga_entry(' ', terminal_color);
        }
    }
    else {
        // Put character in buffer
        const int index = terminal_row * VGA_WIDTH + terminal_column;
        VGA_BUFFER[index] = vga_entry(c, terminal_color);

        // Handle horizontal wrapping
        if (++terminal_column == VGA_WIDTH) {
            terminal_column = 0;
            if (++terminal_row == VGA_HEIGHT) {
                terminal_scroll();
                terminal_row = VGA_HEIGHT - 1;
            }
        }
    }

    terminal_update_cursor();
}

void terminal_write(const char* data) {
    for (int i = 0; data[i] != '\0'; i++) {
        terminal_putchar(data[i]);
    }
}

void terminal_initialize() {
    terminal_row = 0;
    terminal_column = 0;

    //turn blue form the color table into orange
    vga_set_color(1, 255, 127, 0);

    //first number/letter(F) represents the background and the second number/letter(0) represents the text colour
    //0x0 is black black background
    //0x01 was black background and blue forground but now orange forground
    terminal_color = 0x01; 
    
    // Clear the screen with empty spaces
    for (int y = 0; y < VGA_HEIGHT; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            const int index = y * VGA_WIDTH + x;
            VGA_BUFFER[index] = vga_entry(' ', terminal_color);
        }
    }
}