#pragma once

#include <libc/stdint.h>

// Clears the VGA text-mode screen and resets the cursor to (0, 0).
void terminal_init(void);

// Writes a single character to the current cursor position and advances it.
// Handles '\n' (newline) and scrolls when the bottom row is exceeded.
void terminal_putchar(char c);

// Writes a null-terminated string using terminal_putchar.
void terminal_write(const char *str);

// Minimal printf supporting: %c, %s, %d, %u, %x, %%.
// Returns the number of characters written.
int printf(const char *fmt, ...);

// Writes character c with the given VGA colour attribute directly to (row, col).
// Does not move the cursor. Safe to call from game rendering code.
// colour format: bits 7:4 = background, bits 3:0 = foreground.
void terminal_set_cell(int row, int col, char c, uint8_t colour);
