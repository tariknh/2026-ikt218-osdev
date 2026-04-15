#include <libc/stdint.h>
#include <libc/stdarg.h>

extern void terminal_putchar(char c);

static void print_string(const char *s)
{
    while (*s)
        terminal_putchar(*s++);
}

static void print_uint(uint32_t value)
{
    char buf[11];
    int i = 0;

    if (value == 0) {
        terminal_putchar('0');
        return;
    }
    while (value > 0) {
        buf[i++] = '0' + (value % 10);
        value /= 10;
    }
    while (--i >= 0)
        terminal_putchar(buf[i]);
}

static void print_int(int32_t value)
{
    if (value < 0) {
        terminal_putchar('-');
        print_uint((uint32_t)(-value));
    } else {
        print_uint((uint32_t)value);
    }
}

static void print_hex(uint32_t value)
{
    const char *hex = "0123456789abcdef";
    char buf[9];
    int i = 0;

    if (value == 0) {
        terminal_putchar('0');
        return;
    }
    while (value > 0) {
        buf[i++] = hex[value & 0xF];
        value >>= 4;
    }
    while (--i >= 0)
        terminal_putchar(buf[i]);
}

int printf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    int written = 0;
    while (*fmt) {
        if (*fmt == '%') {
            fmt++;
            switch (*fmt) {
            case 'd':
                print_int(va_arg(args, int32_t));
                break;
            case 'u':
                print_uint(va_arg(args, uint32_t));
                break;
            case 'x':
                print_hex(va_arg(args, uint32_t));
                break;
            case 's':
                print_string(va_arg(args, const char *));
                break;
            case 'c':
                terminal_putchar((char)va_arg(args, int));
                break;
            case '%':
                terminal_putchar('%');
                break;
            default:
                terminal_putchar('%');
                terminal_putchar(*fmt);
                break;
            }
        } else {
            terminal_putchar(*fmt);
        }
        fmt++;
        written++;
    }

    va_end(args);
    return written;
}
