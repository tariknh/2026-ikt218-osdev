#ifndef TERMINAL_H
#define TERMINAL_H

#include <libc/stdint.h>

void terminal_clear(void);
void terminal_putchar_at(char character, uint8_t color, int column, int row);
void terminal_write(const char *text);
void terminal_write_dec(uint32_t value);
void terminal_write_hex(uint32_t value);

#endif
