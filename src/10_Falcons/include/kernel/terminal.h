#ifndef UIAOS_KERNEL_TERMINAL_H
#define UIAOS_KERNEL_TERMINAL_H

#include <stdint.h>
#include <stddef.h>

void terminal_initialize(void);
void terminal_clear(void);
void terminal_putchar(char c);
void terminal_write(const char *data);
void terminal_writestring(const char *data);
void terminal_write_hex(uint32_t value);
void terminal_write_dec(uint32_t value);
void printf(const char *format, ...);

#endif
