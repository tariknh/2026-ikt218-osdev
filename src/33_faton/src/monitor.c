#include <monitor.h>
#include <libc/stdint.h>
#include <libc/stdarg.h>

#define VGA_WIDTH  80
#define VGA_HEIGHT 25

static uint16_t* video_memory = (uint16_t*)0xB8000;
static uint8_t cursor_x = 0;
static uint8_t cursor_y = 0;
static uint8_t current_color = 0x07;

static void scroll(void) {
    uint16_t blank = 0x20 | (current_color << 8);
    if (cursor_y >= VGA_HEIGHT) {
        for (int i = 0; i < (VGA_HEIGHT-1) * VGA_WIDTH; i++)
            video_memory[i] = video_memory[i + VGA_WIDTH];
        for (int i = (VGA_HEIGHT-1)*VGA_WIDTH; i < VGA_HEIGHT*VGA_WIDTH; i++)
            video_memory[i] = blank;
        cursor_y = VGA_HEIGHT - 1;
    }
}

void monitor_set_color(uint8_t fg, uint8_t bg) {
    current_color = (bg << 4) | (fg & 0x0F);
}

void monitor_initialize(void) {
    cursor_x = 0;
    cursor_y = 0;
    current_color = 0x07;
    monitor_clear();
}

void monitor_clear(void) {
    uint16_t blank = 0x20 | (current_color << 8);
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++)
        video_memory[i] = blank;
    cursor_x = 0;
    cursor_y = 0;
}

void monitor_put(char c) {
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } else if (c == '\r') {
        cursor_x = 0;
    } else if (c == '\b') {
        if (cursor_x > 0) cursor_x--;
    } else {
        uint16_t attr = current_color << 8;
        video_memory[cursor_y * VGA_WIDTH + cursor_x] = attr | (uint8_t)c;
        cursor_x++;
        if (cursor_x >= VGA_WIDTH) {
            cursor_x = 0;
            cursor_y++;
        }
    }
    scroll();
}

void monitor_write(const char* str) {
    while (*str) monitor_put(*str++);
}

void monitor_write_hex(uint32_t n) {
    monitor_write("0x");
    int started = 0;
    for (int i = 28; i >= 0; i -= 4) {
        uint8_t d = (n >> i) & 0xF;
        if (d || started || i == 0) {
            monitor_put(d < 10 ? '0' + d : 'a' + d - 10);
            started = 1;
        }
    }
}

void monitor_write_dec(uint32_t n) {
    if (n == 0) { monitor_put('0'); return; }
    char buf[10];
    int i = 0;
    while (n > 0) { buf[i++] = '0' + (n % 10); n /= 10; }
    for (int j = i-1; j >= 0; j--) monitor_put(buf[j]);
}

void printf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    while (*format) {
        if (*format == '%') {
            format++;
            if (*format == 'd') {
                int val = va_arg(args, int);
                if (val < 0) { monitor_put('-'); val = -val; }
                monitor_write_dec((uint32_t)val);
            } else if (*format == 'x') {
                monitor_write_hex(va_arg(args, uint32_t));
            } else if (*format == 's') {
                monitor_write(va_arg(args, const char*));
            } else if (*format == 'c') {
                monitor_put((char)va_arg(args, int));
            }
        } else {
            monitor_put(*format);
        }
        format++;
    }
    va_end(args);
}