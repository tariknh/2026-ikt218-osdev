#include "terminal.h"

#define VGA_MEMORY ((volatile uint16_t*)0xB8000)


static size_t             terminal_row;
static size_t             terminal_col;
static uint8_t            terminal_color;
static volatile uint16_t *terminal_buffer;

static uint8_t vga_make_color(vga_color fg, vga_color bg)
{
    return (uint8_t)((uint8_t)fg | ((uint8_t)bg << 4));
}


static uint16_t vga_make_entry(char c, uint8_t color)
{
    return (uint16_t)(uint8_t)c | ((uint16_t)color << 8);
}


static void terminal_scroll(void)
{
    size_t row, col;

    for (row = 1; row < VGA_HEIGHT; row++) {
        for (col = 0; col < VGA_WIDTH; col++) {
            terminal_buffer[(row - 1) * VGA_WIDTH + col] =
                terminal_buffer[row * VGA_WIDTH + col];
        }
    }

    for (col = 0; col < VGA_WIDTH; col++) {
        terminal_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + col] =
            vga_make_entry(' ', terminal_color);
    }

    terminal_row = VGA_HEIGHT - 1;
}


void terminal_init(void)
{
    terminal_row    = 0;
    terminal_col    = 0;
    terminal_color  = vga_make_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    terminal_buffer = VGA_MEMORY;

    size_t row, col;
    for (row = 0; row < VGA_HEIGHT; row++) {
        for (col = 0; col < VGA_WIDTH; col++) {
            terminal_buffer[row * VGA_WIDTH + col] =
                vga_make_entry(' ', terminal_color);
        }
    }
}


void terminal_setcolor(vga_color fg, vga_color bg)
{
    terminal_color = vga_make_color(fg, bg);
}


void terminal_putchar(char c)
{
    if (c == '\n') {
        terminal_col = 0;
        if (++terminal_row == VGA_HEIGHT)
            terminal_scroll();
        return;
    }

    if (c == '\r') {
        terminal_col = 0;
        return;
    }

    if (c == '\t') {
        terminal_col = (terminal_col + 8) & ~(size_t)7;
        if (terminal_col >= VGA_WIDTH) {
            terminal_col = 0;
            if (++terminal_row == VGA_HEIGHT)
                terminal_scroll();
        }
        return;
    }

    terminal_buffer[terminal_row * VGA_WIDTH + terminal_col] =
        vga_make_entry(c, terminal_color);

    if (++terminal_col == VGA_WIDTH) {
        terminal_col = 0;
        if (++terminal_row == VGA_HEIGHT)
            terminal_scroll();
    }
}


void terminal_write_n(const char* data, size_t length)
{
    size_t i;
    for (i = 0; i < length; i++)
        terminal_putchar(data[i]);
}


void terminal_write(const char* str)
{
    size_t i;
    for (i = 0; str[i] != '\0'; i++)
        terminal_putchar(str[i]);
}


void terminal_setpos(uint32_t row, uint32_t col)
{
    if (row < VGA_HEIGHT) terminal_row = row;
    if (col < VGA_WIDTH)  terminal_col = col;
}


void terminal_clear(void)
{
    uint8_t blank = vga_make_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    size_t row, col;
    for (row = 0; row < VGA_HEIGHT; row++)
        for (col = 0; col < VGA_WIDTH; col++)
            terminal_buffer[row * VGA_WIDTH + col] = vga_make_entry(' ', blank);
    terminal_row = 0;
    terminal_col = 0;
}
