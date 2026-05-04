#pragma once

#include "libc/stdint.h"
#include "libc/stddef.h"


typedef enum {
    VGA_COLOR_BLACK         = 0,
    VGA_COLOR_BLUE          = 1,
    VGA_COLOR_GREEN         = 2,
    VGA_COLOR_CYAN          = 3,
    VGA_COLOR_RED           = 4,
    VGA_COLOR_MAGENTA       = 5,
    VGA_COLOR_BROWN         = 6,
    VGA_COLOR_LIGHT_GREY    = 7,
    VGA_COLOR_DARK_GREY     = 8,
    VGA_COLOR_LIGHT_BLUE    = 9,
    VGA_COLOR_LIGHT_GREEN   = 10,
    VGA_COLOR_LIGHT_CYAN    = 11,
    VGA_COLOR_LIGHT_RED     = 12,
    VGA_COLOR_LIGHT_MAGENTA = 13,
    VGA_COLOR_LIGHT_BROWN   = 14,
    VGA_COLOR_WHITE         = 15,
} vga_color;

/* Combine foreground and background into a VGA attribute byte */
uint8_t vga_entry_color(vga_color fg, vga_color bg);

/* Clear the screen and reset the cursor to (0, 0) */
void terminal_initialize(void);

/* Change the active text colour */
void terminal_setcolor(uint8_t color);

/* Write a single character (handles '\n') */
void terminal_putchar(char c);

/* Write a null-terminated string */
void terminal_writestring(const char* str);

/* Minimal printf - supports: %c %s %d %u %x %% */
void printf(const char* fmt, ...);

/* -----------------------------------------------------------------
 * Positioned drawing functions (added for Assignment 6 splash)
 * ----------------------------------------------------------------- */

/* Draw a single character at (x, y) with specific fg/bg colours */
void putCharAt(uint16_t x, uint16_t y, char c, uint8_t fg, uint8_t bg);

/* Fill the entire screen with spaces using the given fg/bg colours */
void fillScreen(uint8_t fg, uint8_t bg);

/* Screen dimensions */
uint16_t getScreenWidth(void);
uint16_t getScreenHeight(void);