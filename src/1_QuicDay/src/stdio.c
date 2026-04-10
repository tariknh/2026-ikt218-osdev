#include "libc/stdio.h"

#include "libc/stdarg.h"

enum {
    VGA_WIDTH = 80,
    VGA_HEIGHT = 25,
    VGA_COLOR = 0x0F
};

static volatile uint16_t* const vga_buffer = (volatile uint16_t*)0xB8000;
static size_t cursor_row = 0;
static size_t cursor_column = 0;

static void console_advance_line(void) {
    cursor_column = 0;
    ++cursor_row;
    if (cursor_row >= VGA_HEIGHT) {
        cursor_row = 0;
    }
}

int putchar(int ic) {
    char character = (char)ic;

    if (character == '\n') {
        console_advance_line();
        return ic;
    }

    vga_buffer[cursor_row * VGA_WIDTH + cursor_column] =
        (uint16_t)(VGA_COLOR << 8) | (uint8_t)character;

    ++cursor_column;
    if (cursor_column >= VGA_WIDTH) {
        console_advance_line();
    }

    return ic;
}

bool print(const char* data, size_t length) {
    for (size_t index = 0; index < length; ++index) {
        putchar((int)data[index]);
    }

    return true;
}

static int print_unsigned(uint32_t value, uint32_t base, bool uppercase) {
    static const char lower_digits[] = "0123456789abcdef";
    static const char upper_digits[] = "0123456789ABCDEF";
    const char* digits = uppercase ? upper_digits : lower_digits;

    char buffer[32];
    int count = 0;
    int index = 0;

    if (value == 0) {
        putchar('0');
        return 1;
    }

    while (value > 0) {
        buffer[index++] = digits[value % base];
        value /= base;
    }

    while (index > 0) {
        putchar(buffer[--index]);
        ++count;
    }

    return count;
}

static int print_signed(int32_t value) {
    int count = 0;
    uint32_t magnitude;

    if (value < 0) {
        putchar('-');
        ++count;
        magnitude = (uint32_t)(-(value + 1)) + 1;
    } else {
        magnitude = (uint32_t)value;
    }

    return count + print_unsigned(magnitude, 10, false);
}

int printf(const char* __restrict__ format, ...) {
    va_list args;
    int written = 0;

    va_start(args, format);

    for (size_t index = 0; format[index] != '\0'; ++index) {
        if (format[index] != '%') {
            putchar((int)format[index]);
            ++written;
            continue;
        }

        ++index;
        char specifier = format[index];

        if (specifier == '\0') {
            break;
        }

        if (specifier == '%') {
            putchar('%');
            ++written;
        } else if (specifier == 'c') {
            char c = (char)va_arg(args, int);
            putchar((int)c);
            ++written;
        } else if (specifier == 's') {
            const char* string = va_arg(args, const char*);
            if (string == 0) {
                string = "(null)";
            }

            for (size_t string_index = 0; string[string_index] != '\0'; ++string_index) {
                putchar((int)string[string_index]);
                ++written;
            }
        } else if (specifier == 'd' || specifier == 'i') {
            written += print_signed(va_arg(args, int32_t));
        } else if (specifier == 'u') {
            written += print_unsigned(va_arg(args, uint32_t), 10, false);
        } else if (specifier == 'x') {
            written += print_unsigned(va_arg(args, uint32_t), 16, false);
        } else if (specifier == 'X') {
            written += print_unsigned(va_arg(args, uint32_t), 16, true);
        } else if (specifier == 'p') {
            uint32_t address = (uint32_t)va_arg(args, void*);
            putchar('0');
            putchar('x');
            written += 2;
            written += print_unsigned(address, 16, false);
        } else {
            putchar('%');
            putchar(specifier);
            written += 2;
        }
    }

    va_end(args);
    return written;
}
