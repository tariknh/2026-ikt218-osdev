#include "terminal.h"

static const size_t VGA_TEXT_WIDTH  = 80;
static const size_t VGA_TEXT_HEIGHT = 25;

static size_t cursor_row;
static size_t cursor_col;
static uint8_t text_color;
static uint16_t* vga_buffer;

static inline uint8_t make_color(uint8_t fg, uint8_t bg)
{
    return (uint8_t)(fg | (bg << 4));
}

static inline uint16_t make_vga_entry(unsigned char ch, uint8_t color)
{
    return (uint16_t)ch | (uint16_t)color << 8;
}

static void put_entry_at(char c, uint8_t color, size_t x, size_t y)
{
    const size_t index = y * VGA_TEXT_WIDTH + x;
    vga_buffer[index] = make_vga_entry((unsigned char)c, color);
}

void terminal_clear(void)
{
    cursor_row = 0;
    cursor_col = 0;

    for (size_t y = 0; y < VGA_TEXT_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_TEXT_WIDTH; x++) {
            put_entry_at(' ', text_color, x, y);
        }
    }
}

void terminal_initialize(void)
{
    vga_buffer = (uint16_t*)0xB8000;
    text_color = make_color(7 /* light grey */, 0 /* black */);
    terminal_clear();
}

static void put_char(char c)
{
    if (c == '\n') {
        cursor_col = 0;
        cursor_row++;
        if (cursor_row >= VGA_TEXT_HEIGHT) cursor_row = 0; // wrap, ingen scrolling
        return;
    }

    put_entry_at(c, text_color, cursor_col, cursor_row);

    cursor_col++;
    if (cursor_col >= VGA_TEXT_WIDTH) {
        cursor_col = 0;
        cursor_row++;
        if (cursor_row >= VGA_TEXT_HEIGHT) cursor_row = 0;
    }
}

void terminal_write(const char* str)
{
    for (size_t i = 0; str[i] != '\0'; i++) {
        put_char(str[i]);
    }
}
void terminal_putc(char c)
{
    put_char(c);
}

