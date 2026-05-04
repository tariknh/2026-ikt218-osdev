#ifndef TERMINAL_H
#define TERMINAL_H

#include "libc/stddef.h"
#include "libc/stdint.h"

// Initialiser terminal-screen
void terminal_initialize(void);

// Write a character to the terminal output
void terminal_putchar(char c);

// Write a string to the terminal output (printf)
void printf(const char* data);
// Write a integer to the terminal output (print_int)
void print_int(uint32_t num);   
// Write a hexadecimal number to the temrinal output (print_hex)
void print_hex(uint32_t num);

void update_hardware_cursor(void);

void terminal_clear(void);

void terminal_draw_entry(char c, uint8_t color, size_t x, size_t y);

#endif