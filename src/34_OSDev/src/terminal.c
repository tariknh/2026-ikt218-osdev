#include "terminal.h"
#include <libc/stdint.h>

//screen is 80 columns x 25 rows (vga text mode), each cell is 2 bytes (character + attribute/color)
#define VGA_WIDTH  80
#define VGA_HEIGHT 25
#define VGA_BUFFER ((uint16_t*)0xB8000) //memory address for writing to screen

//pack character and color into 16-bit VGA entry
static inline uint16_t vga_entry(char c, uint8_t color) {
    return (uint16_t)c | ((uint16_t)color << 8);
}

//background upper 4 bits and foreground lower 4 bits
static inline uint8_t vga_color(uint8_t fg, uint8_t bg) {
    return fg | (bg << 4);
}

static size_t terminal_row;
static size_t terminal_col;
static uint8_t terminal_color;
static uint16_t* terminal_buffer;

void terminal_initialize(void) {
    terminal_row = 0;
    terminal_col = 0;
    terminal_color = vga_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
    terminal_buffer = VGA_BUFFER;

    //fill all cells with blank space to clear screen
    for (size_t row = 0; row < VGA_HEIGHT; row++) {
        for (size_t col = 0; col < VGA_WIDTH; col++) {
            //converts row and column into one number (VGA buffer is a cells list not grid)
            terminal_buffer[row * VGA_WIDTH + col] = vga_entry(' ', terminal_color);
        }
    }
}

void terminal_set_color(uint8_t fg, uint8_t bg) {
    terminal_color = vga_color(fg, bg);
}

//scrolls screen up one row when cursor reaches bottom
//by copying each row one position up and clearing last row for typing space
static void terminal_scroll(void) {
    for (size_t row = 1; row < VGA_HEIGHT; row++) {
        for (size_t col = 0; col < VGA_WIDTH; col++) {
            terminal_buffer[(row - 1) * VGA_WIDTH + col] =
                terminal_buffer[row * VGA_WIDTH + col];
        }
    }
    //clear the last row
    for (size_t col = 0; col < VGA_WIDTH; col++) {
        terminal_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + col] =
            vga_entry(' ', terminal_color);
    }
    terminal_row = VGA_HEIGHT - 1;
}

//writes a single character to the screen at cursors current position
void terminal_write_char(char c) {
    if (c == '\n') {
        terminal_col = 0;
        terminal_row++;
        if (terminal_row >= VGA_HEIGHT) {
            terminal_scroll();
        }
        return;
    }

    terminal_buffer[terminal_row * VGA_WIDTH + terminal_col] =
        vga_entry(c, terminal_color);

    terminal_col++;
    if (terminal_col >= VGA_WIDTH) {
        terminal_col = 0;
        terminal_row++;
        if (terminal_row >= VGA_HEIGHT) {
            terminal_scroll();
        }
    }
}

//writes a string to the screen one character at a time
void terminal_write(const char* str) {
    for (size_t i = 0; str[i] != '\0'; i++) {
        terminal_write_char(str[i]);
    }
}

//prints a positive number, base 10 for normal/decimal numbers, base 16 for hex
static void print_uint(uint32_t value, uint32_t base) {
    char buf[32];
    int i = 0;

    if (value == 0) {
        terminal_write_char('0');
        return;
    }
    //repeatedly divide by base, remainder gives next digit
    while (value > 0) {
        uint32_t digit = value % base;
        buf[i++] = (digit < 10) ? ('0' + digit) : ('a' + digit - 10);
        value /= base;
    }
    //print digits backwards since they were collected in reverse
    for (int j = i - 1; j >= 0; j--) {
        terminal_write_char(buf[j]);
    }
}

//prints negative numbers
static void print_int(int32_t value) {
    if (value < 0) {
        terminal_write_char('-');
        print_uint((uint32_t)(-value), 10);
    } else {
        print_uint((uint32_t)value, 10);
    }
}

//own printf supporting types %c %s %d %u %x %% (cause no stdlib)
int printf(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    for (size_t i = 0; fmt[i] != '\0'; i++) {
        if (fmt[i] != '%') {
            terminal_write_char(fmt[i]);
            continue;
        }
        i++;
        switch (fmt[i]) {
            case 'c':
                terminal_write_char((char)va_arg(args, int));
                break;
            case 's': {
                const char* s = va_arg(args, const char*);
                terminal_write(s ? s : "(null)");
                break;
            }
            case 'd':
                print_int(va_arg(args, int32_t));
                break;
            case 'u':
                print_uint(va_arg(args, uint32_t), 10);
                break;
            case 'x':
                print_uint(va_arg(args, uint32_t), 16);
                break;
            case '%':
                terminal_write_char('%');
                break;
            default:
                terminal_write_char('%');
                terminal_write_char(fmt[i]);
                break;
        }
    }

    va_end(args);
    return 0;
}