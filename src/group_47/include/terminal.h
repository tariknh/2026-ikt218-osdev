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

void print_int(uint32_t num);   

void update_hardware_cursor(void);

#endif