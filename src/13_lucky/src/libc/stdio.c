#include "stdio.h"
#include "stdarg.h"
#include "string.h"
#include "stdint.h"

#define VGA_WIDTH 80
#define VGA_HEIGHT 25

static volatile uint16_t *const VGA_MEMORY = (volatile uint16_t *) 0xB8000;
static const uint8_t VGA_COLOR = 0x0F;
static const char HEX_DIGITS[] = "0123456789abcdef";
static size_t terminal_row;
static size_t terminal_column;
// Stores the length of each line so backspace can move to the previous line after wrapping around
static size_t terminal_line_lengths[VGA_HEIGHT];

static size_t local_strlen(const char *text) {
    size_t length = 0;

    while (text[length] != '\0') {
        length++;
    }

    return length;
}

static int print_int(int value) {
    char digits[10];
    int count = 0;
    unsigned int number = value < 0 ? (unsigned int) -(value + 1) + 1 : (unsigned int) value;

    if (value < 0) {
        putchar('-');
    }

    if (number == 0) {
        putchar('0');
        return value < 0 ? 2 : 1;
    }

    while (number > 0) {
        digits[count++] = (char) ('0' + number % 10);
        number /= 10;
    }

    while (count > 0) {
        putchar(digits[--count]);
    }

    return count + (value < 0 ? 1 : 0);
}

static int print_hex(unsigned int value) {
    char digits[8];
    int count = 0;

    if (value == 0) {
        putchar('0');
        return 1;
    }

    while (value > 0) {
        digits[count++] = HEX_DIGITS[value & 0xF];
        value >>= 4;
    }

    while (count > 0) {
        putchar(digits[--count]);
    }

    return count;
}

void terminal_clear(void) {
    for (size_t row = 0; row < VGA_HEIGHT; row++) {
        for (size_t column = 0; column < VGA_WIDTH; column++) {
            VGA_MEMORY[row * VGA_WIDTH + column] = (uint16_t) ' ' | (uint16_t) VGA_COLOR << 8;
        }
    }

    terminal_row = 0;
    terminal_column = 0;

    for (size_t row = 0; row < VGA_HEIGHT; row++) {
        terminal_line_lengths[row] = 0;
    }
}

int putchar(int c) {
    // Backspace edits the current VGA cell
    if (c == '\b') {
        if (terminal_column > 0) {
            terminal_column--;
            VGA_MEMORY[terminal_row * VGA_WIDTH + terminal_column] = (uint16_t) ' ' | (uint16_t) VGA_COLOR << 8;
            terminal_line_lengths[terminal_row] = terminal_column;
        } else if (terminal_row > 0) {
            terminal_row--;
            terminal_column = terminal_line_lengths[terminal_row];

            if (terminal_column >= VGA_WIDTH) {
                terminal_column = VGA_WIDTH - 1;
            }
        }
        return c;
    }

    // Linebreak increases the row count
    if (c == '\n') {
        terminal_column = 0;
        if (terminal_row < VGA_HEIGHT - 1) {
            terminal_row++;
        }
        terminal_line_lengths[terminal_row] = 0;
        return c;
    }

    // VGA text mode stores the screen as a flat 80-column array of character cells (VGA_WIDTH)
    VGA_MEMORY[terminal_row * VGA_WIDTH + terminal_column] = (uint16_t) c | (uint16_t) VGA_COLOR << 8;
    terminal_column++;
    terminal_line_lengths[terminal_row] = terminal_column;

    if (terminal_column >= VGA_WIDTH) {
        terminal_column = 0;
        if (terminal_row < VGA_HEIGHT - 1) {
            terminal_row++;
        }
        terminal_line_lengths[terminal_row] = 0;
    }

    return c;
}

// Prints to terminal with supported formatting:
// %s for strings
// %c for characters
// %d, %i for signed decimal
// %x for lowercase hexadecimal
int printf(const char *__restrict__ format, ...) {
    va_list args;
    int written = 0;

    va_start(args, format);

    for (size_t i = 0; format[i] != '\0'; i++) {
        if (format[i] == '%' && format[i + 1] == 's') {
            const char *text = va_arg(args, const char *);
            size_t len = local_strlen(text);
            for (size_t ci = 0; ci < len; ci++) {
                putchar(text[ci]);
            }
            written += (int) len;
            i++;
        } else if (format[i] == '%' && format[i + 1] == 'c') {
            putchar(va_arg(args, int));
            written++;
            i++;
        } else if (format[i] == '%' && (format[i + 1] == 'i' || format[i + 1] == 'd')) {
            written += print_int(va_arg(args, int));
            i++;
        } else if (format[i] == '%' && format[i + 1] == 'x') {
            written += print_hex(va_arg(args, unsigned int));
            i++;
        } else {
            putchar(format[i]);
            written++;
        }
    }

    va_end(args);
    return written;
}
