#include <libc/stdarg.h>
#include <libc/stdint.h>

static volatile uint16_t *video = (volatile uint16_t *)0xB8000;
static int cursor = 0;

static void putchar(char c)
{
    if (c == '\n')
    {
        cursor += 80 - (cursor % 80);
        return;
    }

    video[cursor++] = (uint16_t)c | (uint16_t)0x0F00;
}

static void print_string(const char *str)
{
    while (*str)
    {
        putchar(*str++);
    }
}

static void print_decimal(int value)
{
    char buffer[16];
    int i = 0;

    if (value == 0)
    {
        putchar('0');
        return;
    }

    if (value < 0)
    {
        putchar('-');
        value = -value;
    }

    while (value > 0)
    {
        buffer[i++] = '0' + (value % 10);
        value /= 10;
    }

    while (i > 0)
    {
        putchar(buffer[--i]);
    }
}

static void print_hex(uint32_t value)
{
    char *hex = "0123456789ABCDEF";
    print_string("0x");

    for (int i = 7; i >= 0; i--)
    {
        putchar(hex[(value >> (i * 4)) & 0xF]);
    }
}

int printf(const char *format, ...)
{
    va_list args;
    va_start(args, format);

    for (int i = 0; format[i] != '\0'; i++)
    {
        if (format[i] == '%')
        {
            i++;

            if (format[i] == 'd')
            {
                print_decimal(va_arg(args, int));
            }
            else if (format[i] == 'x')
            {
                print_hex(va_arg(args, uint32_t));
            }
            else if (format[i] == 's')
            {
                print_string(va_arg(args, char *));
            }
            else if (format[i] == 'c')
            {
                putchar((char)va_arg(args, int));
            }
            else
            {
                putchar('%');
                putchar(format[i]);
            }
        }
        else
        {
            putchar(format[i]);
        }
    }

    va_end(args);
    return 0;
}

void panic(const char *message)
{
    printf("KERNEL PANIC: %s\n", message);

    while (1)
    {
        __asm__ volatile("hlt");
    }
}