#include "printf.h"
#include <stdarg.h>
#include <stdint.h>

extern void terminal_putchar(char c);

static void print_string(const char* s)
{
    while (*s) {
        terminal_putchar(*s++);
    }
}

static void print_uint(uint32_t val, int base)
{
    char buf[32];
    int i = 31;
    buf[31] = '\0';

    if (val == 0) {
        terminal_putchar('0');
        return;
    }

    while (val > 0 && i > 0) {
        uint32_t digit = val % base;
        buf[--i] = digit < 10 ? '0' + digit : 'a' + (digit - 10);
        val /= base;
    }

    print_string(&buf[i]);
}

static void print_int(int32_t val)
{
    if (val < 0) {
        terminal_putchar('-');
        print_uint((uint32_t)(-val), 10);
    } else {
        print_uint((uint32_t)val, 10);
    }
}

int printf(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    int count = 0;

    while (*fmt) {
        if (*fmt == '%') {
            fmt++;
            switch (*fmt) {
                case 'd': {
                    int32_t val = va_arg(args, int32_t);
                    print_int(val);
                    count++;
                    break;
                }
                case 'u': {
                    uint32_t val = va_arg(args, uint32_t);
                    print_uint(val, 10);
                    count++;
                    break;
                }
                case 'x': {
                    uint32_t val = va_arg(args, uint32_t);
                    print_uint(val, 16);
                    count++;
                    break;
                }
                case 's': {
                    const char* s = va_arg(args, const char*);
                    print_string(s);
                    count++;
                    break;
                }
                case 'c': {
                    char c = (char)va_arg(args, int);
                    terminal_putchar(c);
                    count++;
                    break;
                }
                case '%': {
                    terminal_putchar('%');
                    count++;
                    break;
                }
                default:
                    terminal_putchar('%');
                    terminal_putchar(*fmt);
                    count++;
                    break;
            }
        } else {
            terminal_putchar(*fmt);
            count++;
        }
        fmt++;
    }

    va_end(args);
    return count;
}
