/*
Name: terminal.h
Project: LocOS
Description: This file contains the function declarations for the terminal subsystem of the LocOS kernel.
*/

#ifndef TERMINAL_H
#define TERMINAL_H

#include <libc/stdint.h>

// Set up the VGA terminal
void terminal_init(void);
// Write plain text
void terminal_write(const char *str);
// Write formatted text
void terminal_printf(const char *fmt, ...);
// Clear the screen
void terminal_clear(void);
// Move the cursor to the top left
void terminal_home(void);
// Draw one character at a fixed position
void terminal_put_at(int x, int y, char c);
// Draw one character with a custom color
void terminal_put_colored_at(int x, int y, char c, uint8_t color);
// Return the screen width
int terminal_width(void);
// Return the screen height
int terminal_height(void);
// Keep the bottom rows free for other output
void terminal_reserve_bottom_rows(int rows);

#endif