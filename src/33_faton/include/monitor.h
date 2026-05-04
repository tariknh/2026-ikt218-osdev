#ifndef MONITOR_H
#define MONITOR_H

#include <libc/stdint.h>

#define COLOR_BLACK   0
#define COLOR_BLUE    1
#define COLOR_GREEN   2
#define COLOR_CYAN    3
#define COLOR_RED     4
#define COLOR_MAGENTA 5
#define COLOR_BROWN   6
#define COLOR_WHITE   7
#define COLOR_YELLOW  14

void monitor_initialize(void);
void monitor_clear(void);
void monitor_put(char c);
void monitor_write(const char* str);
void monitor_write_hex(uint32_t n);
void monitor_write_dec(uint32_t n);
void monitor_set_color(uint8_t fg, uint8_t bg);
void printf(const char* format, ...);

#endif