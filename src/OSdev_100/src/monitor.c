#include "../include/monitor.h"
#include "../include/libc/stdarg.h"
#include "../include/libc/stdbool.h"
#include "../include/libc/stdio.h"
#include "../include/io.h"

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define MONITOR_HISTORY_LINES 512

static uint16_t* const vga_buffer = (uint16_t*)0xB8000;
static uint16_t monitor_history[MONITOR_HISTORY_LINES][VGA_WIDTH];
static size_t terminal_row = 0;
static size_t terminal_column = 0;
static size_t viewport_row = 0;
static size_t monitor_left_margin = 0;
static uint8_t monitor_color = COLOR_WHITE;
static void (*monitor_overlay_fn)(void) = 0;

static uint16_t vga_entry(unsigned char character, uint8_t color) {
    return (uint16_t)character | ((uint16_t)color << 8);
}

// Copy the visible window of the scrollback buffer into VGA memory.
static void monitor_render(void) {
    for (size_t row = 0; row < VGA_HEIGHT; row++) {
        size_t history_row = viewport_row + row;
        for (size_t column = 0; column < VGA_WIDTH; column++) {
            if (history_row < MONITOR_HISTORY_LINES) {
                vga_buffer[row * VGA_WIDTH + column] = monitor_history[history_row][column];
            } else {
                vga_buffer[row * VGA_WIDTH + column] = vga_entry(' ', monitor_color);
            }
        }
    }

    if (monitor_overlay_fn) {
        monitor_overlay_fn();
    }
}

static void monitor_clear_row(size_t row) {
    for (size_t column = 0; column < VGA_WIDTH; column++) {
        monitor_history[row][column] = vga_entry(' ', monitor_color);
    }
}

static void monitor_shift_history_up(void) {
    for (size_t row = 1; row < MONITOR_HISTORY_LINES; row++) {
        for (size_t column = 0; column < VGA_WIDTH; column++) {
            monitor_history[row - 1][column] = monitor_history[row][column];
        }
    }
    monitor_clear_row(MONITOR_HISTORY_LINES - 1);
}

static size_t monitor_bottom_viewport(void) {
    if (terminal_row + 1 > VGA_HEIGHT) {
        return terminal_row + 1 - VGA_HEIGHT;
    }
    return 0;
}

static bool monitor_is_following_bottom(void) {
    return viewport_row == monitor_bottom_viewport();
}

// Advance to the next line while keeping the viewport pinned to the bottom when needed.
static void monitor_newline(void) {
    bool follow_bottom = monitor_is_following_bottom();

    terminal_column = monitor_left_margin;
    terminal_row++;

    if (terminal_row >= MONITOR_HISTORY_LINES) {
        monitor_shift_history_up();
        terminal_row = MONITOR_HISTORY_LINES - 1;
        if (viewport_row > 0) {
            viewport_row--;
        }
    }

    monitor_clear_row(terminal_row);

    if (follow_bottom) {
        viewport_row = monitor_bottom_viewport();
    }
}

void monitor_initialize(void) {
    terminal_row = 0;
    terminal_column = 0;
    viewport_row = 0;
    monitor_left_margin = 0;
    monitor_overlay_fn = 0;

    for (size_t row = 0; row < MONITOR_HISTORY_LINES; row++) {
        monitor_clear_row(row);
    }

    monitor_render();
}

// Write one character into the history buffer, then redraw the visible region.
void monitor_put(char c) {
    if (c == '\n') {
        monitor_newline();
    } else {
        monitor_history[terminal_row][terminal_column] = vga_entry((unsigned char)c, monitor_color);
        terminal_column++;
    }

    if (terminal_column >= VGA_WIDTH) {
        monitor_newline();
    }

    if (monitor_is_following_bottom()) {
        viewport_row = monitor_bottom_viewport();
    }

    monitor_render();
}

void monitor_write(const char* data) {
    if (!data) {
        return;
    }

    while (*data) {
        monitor_put(*data++);
    }
}

void monitor_write_dec(int value) {
    char buffer[12];
    unsigned int number;
    int index = 0;

    if (value == 0) {
        monitor_put('0');
        return;
    }

    if (value < 0) {
        monitor_put('-');
        number = (unsigned int)(-value);
    } else {
        number = (unsigned int)value;
    }

    while (number > 0) {
        buffer[index++] = (char)('0' + (number % 10));
        number /= 10;
    }

    while (index > 0) {
        monitor_put(buffer[--index]);
    }
}

void monitor_write_hex(uint32_t value) {
    char buffer[8];
    int index = 0;

    if (value == 0) {
        monitor_put('0');
        return;
    }

    while (value > 0) {
        uint32_t digit = value & 0xF;
        buffer[index++] = (char)(digit < 10 ? ('0' + digit) : ('a' + digit - 10));
        value >>= 4;
    }

    while (index > 0) {
        monitor_put(buffer[--index]);
    }
}

void monitor_scroll_up(void) {
    if (viewport_row > 0) {
        viewport_row--;
        monitor_render();
    }
}

void monitor_scroll_down(void) {
    size_t bottom = monitor_bottom_viewport();
    if (viewport_row < bottom) {
        viewport_row++;
        monitor_render();
    }
}

void monitor_set_cursor(size_t row, size_t column) {
    if (row >= VGA_HEIGHT) {
        row = VGA_HEIGHT - 1;
    }

    if (column >= VGA_WIDTH) {
        column = VGA_WIDTH - 1;
    }

    terminal_row = viewport_row + row;
    terminal_column = column;
    monitor_render();
}

void monitor_set_left_margin(size_t column) {
    if (column >= VGA_WIDTH) {
        column = 0;
    }

    monitor_left_margin = column;
    if (terminal_column < monitor_left_margin) {
        terminal_column = monitor_left_margin;
    }
}

void monitor_set_overlay(void (*overlay_fn)(void)) {
    monitor_overlay_fn = overlay_fn;
    monitor_render();
}

void monitor_set_color(uint8_t color) {
    monitor_color = color;
}

uint8_t monitor_get_color(void) {
    return monitor_color;
}

void monitor_show_cursor(void) {
    outb(0x3D4, 0x0A);
    outb(0x3D5, 0x0E);
    outb(0x3D4, 0x0B);
    outb(0x3D5, 0x0F);
}

void monitor_hide_cursor(void) {
    outb(0x3D4, 0x0A);
    outb(0x3D5, 0x20);
}

int putchar(int ic) {
    monitor_put((char)ic);
    return ic;
}

bool print(const char* data, size_t length) {
    if (!data) {
        return false;
    }

    for (size_t i = 0; i < length; i++) {
        monitor_put(data[i]);
    }

    return true;
}

int printf(const char* format, ...) {
    va_list args;
    int written = 0;

    if (!format) {
        return 0;
    }

    va_start(args, format);

    // Walk the format string one character at a time and expand supported specifiers.
    while (*format) {
        if (*format != '%') {
            monitor_put(*format++);
            written++;
            continue;
        }

        format++;

        switch (*format) {
            case 'c': {
                char c = (char)va_arg(args, int);
                monitor_put(c);
                written++;
                break;
            }
            case 's': {
                const char* s = va_arg(args, const char*);
                if (!s) {
                    s = "(null)";
                }
                while (*s) {
                    monitor_put(*s++);
                    written++;
                }
                break;
            }
            case 'd': {
                int value = va_arg(args, int);
                monitor_write_dec(value);
                break;
            }
            case 'x': {
                uint32_t value = va_arg(args, uint32_t);
                monitor_write_hex(value);
                break;
            }
            case '%': {
                monitor_put('%');
                written++;
                break;
            }
            default: {
                monitor_put('%');
                written++;
                if (*format) {
                    monitor_put(*format);
                    written++;
                }
                break;
            }
        }

        if (*format) {
            format++;
        }
    }

    va_end(args);
    return written;
}
