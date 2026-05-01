#pragma once
#include <stdbool.h>

#include "libc/stdint.h"

/**
 * VGA colours
 */
enum vga_text_color {
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


#define VGA_TEXT_WIDTH 80
#define VGA_TEXT_HEIGHT 25
#define VGA_TEXT_MEMORY 0xB8000

// VGA: Video Graphics Array

/**
 * Initialize the vga_terminal
 */
void vga_text_initialise(void);

/**
 * Clear the VGA
 */
void vga_text_clear_screen(void);

/**
 * Set the color of future printed characters
 * @param color color to set
 */
void vga_text_setcolor(uint8_t color);

// NOTE: We can make nice TUI with this prolly

/** Sets given char with given colour at given position
 * @param c
 * @param color
 * @param x x-coordinate
 * @param y y-coordinate
 */
void vga_text_putentryat(char c, uint8_t color, size_t x, size_t y);

/** Put a char to the terminal
 * @param c char to put
 */
void vga_text_putchar(char c);

/** Writes text to the VGA
 * @param data
 * @param size
 */
void vga_text_write(const char* data, size_t size);

/** Writes text to terminal
 * @param data
 */
void vga_text_writestring(const char* data);

/**
 * Scrolls the VGA terminal down one line
 */
void vga_text_scroll(void);

/**
 * Reads a character and its color from a coordinate
 */
uint16_t vga_text_get_entry_at(size_t x, size_t y);

/**
 * Read the current terminal color
 */
uint8_t vga_text_get_color(void);

/**
 * Get current cursor position
 */
void vga_text_get_cursor_position(size_t* x, size_t* y);

/**
 * Disable the hardware cursor
 */
void vga_text_disable_cursor();

/**
 * Enable debug mode - copies all VGA output to serial port 0xE9
 */
void vga_text_enable_debug_serial(bool enable);

#define vga_text_entry_color(fg, bg) (fg | bg << 3)
