#include "stdio.h"
#include "stdarg.h"
#include "string.h"
#include "stdint.h"

static volatile uint16_t *const VGA_MEMORY = (volatile uint16_t *) 0xB8000;
static const uint8_t VGA_COLOR = 0x0F;
static const size_t VGA_WIDTH = 80;
static size_t terminal_row;
static size_t terminal_column;

static size_t local_strlen(const char *text) {
    size_t length = 0;

    while (text[length] != '\0') {
        length++;
    }

    return length;
}

int putchar(int c) {
    if (c == '\n') {
        terminal_column = 0;
        terminal_row++;
        return c;
    }

    // VGA text mode stores the screen as a flat 80-column array of character cells.
    VGA_MEMORY[terminal_row * VGA_WIDTH + terminal_column] = (uint16_t) c | (uint16_t) VGA_COLOR << 8;
    terminal_column++;

    if (terminal_column >= VGA_WIDTH) {
        terminal_column = 0;
        terminal_row++;
    }

    return c;
}

int printf(const char *__restrict__ format, ...) {
    va_list args;
    int written = 0;

    va_start(args, format);

    for (size_t i = 0; format[i] != '\0'; i++) {
        // This tiny printf currently supports plain characters and %s strings.
        if (format[i] == '%' && format[i + 1] == 's') {
            const char *text = va_arg(args, const char *);
            size_t len = local_strlen(text);
            for (size_t ci = 0; ci < len; ci++) {
                putchar(text[ci]);
            }
            written += (int) len;
            i++;
        } else {
            putchar(format[i]);
            written++;
        }
    }

    va_end(args);
    return written;
}
