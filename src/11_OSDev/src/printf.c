#include "libc/stdbool.h"
#include "libc/stdint.h"
#include "libc/stdarg.h"
#include "libc/stdio.h"
#include "terminal.h"


int putchar(int ic)
{
    terminal_putchar((char)ic);
    return ic;
}


bool print(const char* data, size_t length)
{
    terminal_write_n(data, length);
    return true;
}

/*  internal helper  */


static size_t uint_to_str(unsigned int value, char* buf, int base, int uppercase)
{
    static const char lo[] = "0123456789abcdef";
    static const char hi[] = "0123456789ABCDEF";
    const char* digits = uppercase ? hi : lo;

    char   tmp[33]; 
    size_t len = 0;

    if (value == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return 1;
    }

    while (value) {
        tmp[len++] = digits[value % (unsigned int)base];
        value      /= (unsigned int)base;
    }

    
    size_t i;
    for (i = 0; i < len; i++)
        buf[i] = tmp[len - 1 - i];
    buf[len] = '\0';

    return len;
}

/*  printf  */
int printf(const char* __restrict__ format, ...)
{
    va_list args;
    va_start(args, format);

    int  written = 0;
    char buf[33]; 
    size_t i;

    for (i = 0; format[i] != '\0'; i++) {

        
        if (format[i] != '%') {
            putchar(format[i]);
            written++;
            continue;
        }
        i++;

        switch (format[i]) {

            case 'c': {
                char c = (char)va_arg(args, int);
                putchar(c);
                written++;
                break;
            }

            case 's': {
                const char* s = va_arg(args, const char*);
                if (!s)
                    s = "(null)";
                size_t j;
                for (j = 0; s[j] != '\0'; j++) {
                    putchar(s[j]);
                    written++;
                }
                break;
            }

            case 'd': {
                int val = va_arg(args, int);
                if (val < 0) {
                    putchar('-');
                    written++;
                    val = (int)(~(unsigned int)val + 1u);
                }
                size_t len = uint_to_str((unsigned int)val, buf, 10, 0);
                print(buf, len);
                written += (int)len;
                break;
            }

            case 'u': {
                unsigned int val = va_arg(args, unsigned int);
                size_t len = uint_to_str(val, buf, 10, 0);
                print(buf, len);
                written += (int)len;
                break;
            }

            case 'x': {
                unsigned int val = va_arg(args, unsigned int);
                size_t len = uint_to_str(val, buf, 16, 0);
                print(buf, len);
                written += (int)len;
                break;
            }

            case 'X': {
                unsigned int val = va_arg(args, unsigned int);
                size_t len = uint_to_str(val, buf, 16, 1);
                print(buf, len);
                written += (int)len;
                break;
            }

            case '%': {
                putchar('%');
                written++;
                break;
            }

            case '\0':
                goto done;

            default: {
                putchar('%');
                putchar(format[i]);
                written += 2;
                break;
            }
        }
    }

done:
    va_end(args);
    return written;
}
