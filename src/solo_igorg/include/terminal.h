#ifndef TERMINAL_H
#define TERMINAL_H

#include <libc/stdint.h>

void terminal_initialize(void);
void terminal_putchar(char character);
void terminal_write(const char* data);

/*
 * Helper functions for printing numbers in the kernel.
 */
void terminal_write_dec(uint32_t value);
void terminal_write_hex(uint32_t value);

#endif
