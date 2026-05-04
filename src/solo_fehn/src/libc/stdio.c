/*
 * stdio.c - putchar, print, printf
 *
 * A minimal printf supporting the format specifiers we actually need:
 *   %s   null-terminated C string
 *   %d   signed decimal integer
 *   %i   same as %d
 *   %x   unsigned hexadecimal (lowercase)
 *   %c   single character
 *   %%   literal '%'
 *
 * No width/precision/flags. Plenty for a kernel banner.
 */

#include <libc/stdio.h>
#include <libc/stdarg.h>
#include <libc/stdbool.h>
#include <terminal.h>

int putchar(int ic) {
    terminal_putchar((char)ic);
    return ic;
}

bool print(const char* data, size_t length) {
    terminal_write(data, length);
    return true;
}

/* Helper: print an integer in the given base (10 or 16), with optional sign. */
static void print_unsigned(unsigned int value, int base) {
    static const char digits[] = "0123456789abcdef";
    char buf[32];
    int i = 0;

    if (value == 0) {
        putchar('0');
        return;
    }
    while (value > 0) {
        buf[i++] = digits[value % (unsigned int)base];
        value /= (unsigned int)base;
    }
    while (i > 0) {
        i--;
        putchar(buf[i]);
    }
}

static void print_signed(int value) {
    if (value < 0) {
        putchar('-');
        /* Cast carefully so INT_MIN doesn't break us. */
        print_unsigned((unsigned int)(-(value + 1)) + 1u, 10);
    } else {
        print_unsigned((unsigned int)value, 10);
    }
}

int printf(const char* format, ...) {
    va_list args;
    va_start(args, format);

    int written = 0;
    while (*format != '\0') {
        if (*format != '%') {
            putchar(*format);
            written++;
            format++;
            continue;
        }

        /* We saw a '%'. Look at the next character. */
        format++;
        switch (*format) {
            case 's': {
                const char* s = va_arg(args, const char*);
                while (*s) {
                    putchar(*s);
                    s++;
                    written++;
                }
                break;
            }
            case 'd':
            case 'i': {
                int v = va_arg(args, int);
                print_signed(v);
                written++;
                break;
            }
            case 'x': {
                unsigned int v = va_arg(args, unsigned int);
                print_unsigned(v, 16);
                written++;
                break;
            }
            case 'c': {
                /* char promotes to int when passed through ... */
                char c = (char)va_arg(args, int);
                putchar(c);
                written++;
                break;
            }
            case '%':
                putchar('%');
                written++;
                break;
            default:
                /* Unknown specifier - just print it literally. */
                putchar('%');
                putchar(*format);
                written += 2;
                break;
        }
        if (*format != '\0') {
            format++;
        }
    }

    va_end(args);
    return written;
}
