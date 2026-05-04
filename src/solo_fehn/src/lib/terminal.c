/*
 * terminal.c - VGA text-mode terminal
 *
 * Updated for Assignment 3 to also handle the backspace character so the
 * keyboard's backspace key actually erases the previous character.
 */

#include <terminal.h>
#include <libc/string.h>

#define VGA_WIDTH   80
#define VGA_HEIGHT  25

#define VGA_DEFAULT_COLOUR  ((uint16_t)0x0F << 8)   /* white on black */

static volatile uint16_t* const vga_buffer = (volatile uint16_t*)0xB8000;

static size_t cursor_row;
static size_t cursor_col;

static void put_at(size_t row, size_t col, char c) {
    vga_buffer[row * VGA_WIDTH + col] = (uint16_t)(uint8_t)c | VGA_DEFAULT_COLOUR;
}

void terminal_initialize(void) {
    cursor_row = 0;
    cursor_col = 0;
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            put_at(y, x, ' ');
        }
    }
}

static void terminal_scroll(void) {
    for (size_t y = 1; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            vga_buffer[(y - 1) * VGA_WIDTH + x] = vga_buffer[y * VGA_WIDTH + x];
        }
    }
    for (size_t x = 0; x < VGA_WIDTH; x++) {
        put_at(VGA_HEIGHT - 1, x, ' ');
    }
    cursor_row = VGA_HEIGHT - 1;
}

void terminal_putchar(char c) {
    if (c == '\n') {
        cursor_col = 0;
        cursor_row++;
    } else if (c == '\r') {
        cursor_col = 0;
    } else if (c == '\t') {
        cursor_col = (cursor_col + 8) & ~(size_t)7;
    } else if (c == '\b') {
        if (cursor_col > 0) {
            cursor_col--;
            put_at(cursor_row, cursor_col, ' ');
        } else if (cursor_row > 0) {
            cursor_row--;
            cursor_col = VGA_WIDTH - 1;
            put_at(cursor_row, cursor_col, ' ');
        }
    } else {
        put_at(cursor_row, cursor_col, c);
        cursor_col++;
    }

    if (cursor_col >= VGA_WIDTH) {
        cursor_col = 0;
        cursor_row++;
    }
    if (cursor_row >= VGA_HEIGHT) {
        terminal_scroll();
    }
}

void terminal_write(const char* str, size_t len) {
    for (size_t i = 0; i < len; i++) {
        terminal_putchar(str[i]);
    }
}

void terminal_writestring(const char* str) {
    terminal_write(str, strlen(str));
}
