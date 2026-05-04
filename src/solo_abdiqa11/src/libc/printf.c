#include <stdarg.h>
#include <stdint.h>
#include "terminal.h"
#include "libc/stdio.h"
#include "libc/panic.h"

static void putc_local(char c) {
    char s[2] = {c, 0};
    terminal_write(s);
}

static void puts_local(const char *s) {
    terminal_write(s);
}

static void put_hex(uint32_t v) {
    puts_local("0x");
    for (int i = 7; i >= 0; i--) {
        uint8_t nib = (v >> (i * 4)) & 0xF;
        putc_local(nib < 10 ? ('0' + nib) : ('a' + (nib - 10)));
    }
}

static void put_dec(uint32_t n) {
    char buf[11];
    int i = 0;
    if (n == 0) { putc_local('0'); return; }
    while (n && i < 10) { buf[i++] = '0' + (n % 10); n /= 10; }
    while (i--) putc_local(buf[i]);
}

// Supports: %s %c %d %u %x  (enough for the assignment prints)
int printf(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    for (const char *p = fmt; *p; p++) {
        if (*p != '%') { putc_local(*p); continue; }
        p++;
        if (!*p) break;

        switch (*p) {
            case '%': putc_local('%'); break;
            case 's': puts_local(va_arg(ap, const char*)); break;
            case 'c': putc_local((char)va_arg(ap, int)); break;
            case 'd': put_dec((uint32_t)va_arg(ap, int)); break;
            case 'u': put_dec(va_arg(ap, uint32_t)); break;
            case 'x': put_hex(va_arg(ap, uint32_t)); break;
            default:
                // unknown specifier, print it raw
                putc_local('%');
                putc_local(*p);
                break;
        }
    }

    va_end(ap);
    return 0;
}

void panic(const char *msg) {
    terminal_write("PANIC: ");
    terminal_write(msg);
    terminal_write("\n");
    for (;;) asm volatile("cli; hlt");
}
