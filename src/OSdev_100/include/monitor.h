#pragma once

#include "libc/stddef.h"
#include "libc/stdint.h"

#define COLOR_BLACK       0x00
#define COLOR_DARK_GRAY   0x08
#define COLOR_LIGHT_GRAY  0x07
#define COLOR_WHITE       0x0F
#define COLOR_YELLOW      0x0E
#define COLOR_LIGHT_GREEN 0x0A

void monitor_initialize(void);
void monitor_put(char c);
void monitor_write(const char* data);
void monitor_write_dec(int value);
void monitor_write_hex(uint32_t value);
void monitor_scroll_up(void);
void monitor_scroll_down(void);
void monitor_set_cursor(size_t row, size_t column);
void monitor_set_left_margin(size_t column);
void monitor_set_overlay(void (*overlay_fn)(void));
void monitor_set_color(uint8_t color);
uint8_t monitor_get_color(void);
void monitor_show_cursor(void);
void monitor_hide_cursor(void);
