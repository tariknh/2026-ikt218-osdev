#ifndef TERMINAL_H
#define TERMINAL_H

#include <libc/stddef.h>
#include <libc/stdint.h>

void terminal_initialize(void);
void terminal_clear(void);
void terminal_write(const char* str);
void terminal_putc(char c);

#endif
