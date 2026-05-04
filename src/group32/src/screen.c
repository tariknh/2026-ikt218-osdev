#include "screen.h"

static unsigned short* const video_memory = (unsigned short*)0xB8000; // VGA text mode memory address

// Cursor position
static int cursor_x = 0;
static int cursor_y = 0;
static const unsigned char color = 0x0F; // White text on black background

// Scrolls the screen up if needed
static void screen_scroll(void) {
    if (cursor_y < 25) {
        return;
    }

    int i;
    // Move all lines one row up
    for (i = 0; i < 24 * 80; i++) {
        video_memory[i] = video_memory[i + 80];
    }
    // Clear last line
    for (i = 24 * 80; i < 25 * 80; i++) {
        video_memory[i] = ((unsigned short)color << 8) | ' ';
    }

    cursor_y = 24;     // Keep cursor on last line

}
// Clears the screen
void screen_clear(void) {
    int i;
    // Fill screen with spaces
    for (i = 0; i < 80 * 25; i++) {
        video_memory[i] = ((unsigned short)color << 8) | ' ';
    }
    // Reset cursor
    cursor_x = 0;
    cursor_y = 0;
}

// Prints one character
void screen_putc(char c) {
    // Handle newline
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
        screen_scroll();
        return;
    }
    // Handle backspace
    if (c == '\b') {
        if (cursor_x > 0) {
            cursor_x--;
        } else if (cursor_y > 0) {
            cursor_y--;
            cursor_x = 79;
        }

        video_memory[cursor_y * 80 + cursor_x] = ((unsigned short)color << 8) | ' ';
        return;
    }
    // Write character to video memory
    video_memory[cursor_y * 80 + cursor_x] = ((unsigned short)color << 8) | c;
    cursor_x++;
    // Move to next line if needed
    if (cursor_x >= 80) {
        cursor_x = 0;
        cursor_y++;
    }

    screen_scroll();
}
// Prints a string
void screen_write(const char* str) {
    int i = 0;
    // Print characters until null terminator
    while (str[i]) {
        screen_putc(str[i]);
        i++;
    }
}
// Prints a number as hexadecimal
void screen_write_hex(unsigned int n) {
    char hex[] = "0123456789ABCDEF";
    int i;

    screen_write("0x");
    // Print 8 hex digits
    for (i = 28; i >= 0; i -= 4) {
        screen_putc(hex[(n >> i) & 0xF]);
    }
}
// Prints a number as decimal
void screen_write_dec(unsigned int n) {
    char buf[16];
    int i = 0;
    // Handle zero
    if (n == 0) {
        screen_putc('0');
        return;
    }
    // Convert digits in reverse order
    while (n > 0) {
        buf[i++] = '0' + (n % 10);
        n /= 10;
    }
    // Print digits in correct order
    while (i--) {
        screen_putc(buf[i]);
    }
}