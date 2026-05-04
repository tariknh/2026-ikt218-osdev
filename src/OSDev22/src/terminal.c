#include "terminal.h"
#include "libc/stdarg.h"

/*
 * terminal.c - VGA text-mode terminal driver
 */

#define VGA_WIDTH  80
#define VGA_HEIGHT 25

#define VGA_MEMORY ((uint16_t*)0xB8000)

static size_t   terminal_row;
static size_t   terminal_col;
static uint8_t  terminal_color;
static uint16_t *terminal_buffer;

uint8_t vga_entry_color(vga_color fg, vga_color bg)
{
    return (uint8_t)(fg | (bg << 4));
}

static uint16_t vga_entry(unsigned char c, uint8_t color)
{
    return (uint16_t)c | ((uint16_t)color << 8);
}

static void terminal_scroll(void)
{
    for (size_t y = 1; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            terminal_buffer[(y - 1) * VGA_WIDTH + x] =
                terminal_buffer[y * VGA_WIDTH + x];
        }
    }

    for (size_t x = 0; x < VGA_WIDTH; x++) {
        terminal_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + x] =
            vga_entry(' ', terminal_color);
    }

    terminal_row = VGA_HEIGHT - 1;
}

void terminal_initialize(void)
{
    terminal_row    = 0;
    terminal_col    = 0;
    terminal_color  = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    terminal_buffer = VGA_MEMORY;

    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            terminal_buffer[y * VGA_WIDTH + x] = vga_entry(' ', terminal_color);
        }
    }
}

void terminal_setcolor(uint8_t color)
{
    terminal_color = color;
}

void terminal_putchar(char c)
{
    if (c == '\n') {
        terminal_col = 0;
        if (++terminal_row >= VGA_HEIGHT) {
            terminal_scroll();
        }
        return;
    }

    if (c == '\b') {
        if (terminal_col > 0) {
            terminal_col--;
            terminal_buffer[terminal_row * VGA_WIDTH + terminal_col] =
                vga_entry(' ', terminal_color);
        }
        return;
    }

    terminal_buffer[terminal_row * VGA_WIDTH + terminal_col] =
        vga_entry((unsigned char)c, terminal_color);

    if (++terminal_col >= VGA_WIDTH) {
        terminal_col = 0;
        if (++terminal_row >= VGA_HEIGHT) {
            terminal_scroll();
        }
    }
}

void terminal_writestring(const char* str)
{
    for (size_t i = 0; str[i] != '\0'; i++) {
        terminal_putchar(str[i]);
    }
}

static void print_uint(uint32_t value, uint32_t base)
{
    static const char digits[] = "0123456789abcdef";
    char buf[32];
    int  pos = 0;

    if (value == 0) {
        terminal_putchar('0');
        return;
    }

    while (value > 0) {
        buf[pos++] = digits[value % base];
        value /= base;
    }

    while (pos > 0) {
        terminal_putchar(buf[--pos]);
    }
}

void printf(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    for (size_t i = 0; fmt[i] != '\0'; i++) {
        if (fmt[i] != '%') {
            terminal_putchar(fmt[i]);
            continue;
        }

        i++;
        switch (fmt[i]) {
            case 'c':
                terminal_putchar((char)va_arg(args, int));
                break;

            case 's': {
                const char* s = va_arg(args, const char*);
                terminal_writestring(s ? s : "(null)");
                break;
            }

            case 'd': {
                int32_t val = va_arg(args, int32_t);
                if (val < 0) {
                    terminal_putchar('-');
                    print_uint((uint32_t)(-val), 10);
                } else {
                    print_uint((uint32_t)val, 10);
                }
                break;
            }

            case 'u':
                print_uint(va_arg(args, uint32_t), 10);
                break;

            case 'x':
                print_uint(va_arg(args, uint32_t), 16);
                break;

            case '%':
                terminal_putchar('%');
                break;

            default:
                terminal_putchar('%');
                terminal_putchar(fmt[i]);
                break;
        }
    }

    va_end(args);
}

/* -------------------------------------------------------------------------
 * Positioned drawing functions (added for Assignment 6 splash screen)
 * ---------------------------------------------------------------------- */

/* Draw a single character at (x, y) with specific fg/bg colours */
void putCharAt(uint16_t x, uint16_t y, char c, uint8_t fg, uint8_t bg)
{
    if (x >= VGA_WIDTH || y >= VGA_HEIGHT) return;

    uint8_t attr = (uint8_t)((fg & 0x0F) | ((bg & 0x0F) << 4));
    VGA_MEMORY[y * VGA_WIDTH + x] = vga_entry((unsigned char)c, attr);
}

/* Fill the entire screen with spaces using the given fg/bg colours */
void fillScreen(uint8_t fg, uint8_t bg)
{
    uint8_t attr = (uint8_t)((fg & 0x0F) | ((bg & 0x0F) << 4));
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            VGA_MEMORY[y * VGA_WIDTH + x] = vga_entry(' ', attr);
        }
    }
    terminal_row = 0;
    terminal_col = 0;
}

uint16_t getScreenWidth(void)  { return VGA_WIDTH;  }
uint16_t getScreenHeight(void) { return VGA_HEIGHT; }