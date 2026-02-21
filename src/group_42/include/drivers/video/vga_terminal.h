#pragma once
#include "libc/stdint.h"
enum vga_color {
  VGA_COLOR_BLACK = 0,
  VGA_COLOR_BLUE = 1,
  VGA_COLOR_GREEN = 2,
  VGA_COLOR_CYAN = 3,
  VGA_COLOR_RED = 4,
  VGA_COLOR_MAGENTA = 5,
  VGA_COLOR_BROWN = 6,
  VGA_COLOR_LIGHT_GREY = 7,
  VGA_COLOR_DARK_GREY = 8,
  VGA_COLOR_LIGHT_BLUE = 9,
  VGA_COLOR_LIGHT_GREEN = 10,
  VGA_COLOR_LIGHT_CYAN = 11,
  VGA_COLOR_LIGHT_RED = 12,
  VGA_COLOR_LIGHT_MAGENTA = 13,
  VGA_COLOR_LIGHT_BROWN = 14,
  VGA_COLOR_WHITE = 15,
};


#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_MEMORY 0xB8000

/// Initializes to print to VGA terminal
void vga_terminal_initialise(void);

/// Sets the terminal colour
/// @param color
void vga_terminal_setcolor(uint8_t color);

// NOTE: We can make nice TUI with this prolly
/// Sets given char with given colour at given position
/// @param c
/// @param color
/// @param x x-coordinate
/// @param y y-coordinate
void vga_terminal_putentryat(char c, uint8_t color, size_t x, size_t y);

/// Put a char to the terminal
/// @param c char to put
void vga_terminal_putchar(char c);
/// Writes data to terminal
/// @param data
/// @param size
void vga_terminal_write(const char* data, size_t size);

/// Writes data to terminal
/// @param data
void vga_terminal_writestring(const char* data);

/// Scrolls the terminal
void vga_terminal_scroll(void);

#define vga_entry_color(fg, bg) (fg | bg << 3)
