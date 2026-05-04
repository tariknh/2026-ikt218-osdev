#include "terminal.h"
#include "common.h"

static const int VGA_WIDTH = 80;
static const int VGA_HEIGHT = 25;
static uint16_t* const VGA_BUFFER = (uint16_t*) 0xB8000;

int terminal_row;
int terminal_column;
static uint8_t terminal_color;

static uint16_t screen_snapshot[80 * 25];
static int snapshot_row;
static int snapshot_col;

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
void terminal_move_left() {
    if (terminal_column > 0) {
        terminal_column--;
        terminal_update_cursor();
    }
} 

void terminal_move_right() {
    if (terminal_column < 79) {
        terminal_column++;
        terminal_update_cursor();
    }
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
void terminal_write_dec(uint32_t v) {
    if (v == 0) { terminal_putchar('0'); return; }
    char buf[11]; int i = 10;
    buf[i] = '\0';
    while (v) { buf[--i] = '0' + (v % 10); v /= 10; }
    terminal_write(buf + i);
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

void terminal_save_screen() {
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        screen_snapshot[i] = VGA_BUFFER[i];
    }
    snapshot_row = terminal_row;
    snapshot_col = terminal_column;
}

void terminal_restore_screen() {
    // 1. Re-apply the orange palette entry (wiped by mode switch)
    vga_set_color(1, 255, 127, 0);

    // 2. Copy the saved content back to the VGA text buffer
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        VGA_BUFFER[i] = screen_snapshot[i];
    }

    // 3. Restore cursor position
    terminal_row    = snapshot_row;
    terminal_column = snapshot_col;
    terminal_update_cursor();
}

void terminal_refresh_line(char* buffer_from_cursor) {
    // 1. Save the starting position
    int start_x = terminal_column;
    int start_y = terminal_row;

    // 2. Print the current buffer content from the cursor forward
    terminal_write(buffer_from_cursor);

    // 3. Clear the "Tail"
    // We print one extra space to catch the 'ghost' character from backspace.
    // If you want to be safer, you can clear to the end of the line.
   
    terminal_putchar(' ');


    // 4. Restore the cursor to the "logical" typing position
    // (Right after the character just typed, or at the same spot for backspace)
    terminal_column = start_x;
    terminal_row = start_y;
    terminal_update_cursor();
}