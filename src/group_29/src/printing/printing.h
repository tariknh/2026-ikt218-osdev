#pragma once

#include "../vga_text_mode_interface/vga_text_mode_interface.h"
#include "../memory/memory_macros.h"
#include "libc/stdint.h"

#define TEMP VGA_TERMINAL_WIDTH * sizeof(uint16_t) * 2

extern struct VgaTextModeInterface main_interface;

void init_vga_interface_for_printing();
void scroll_screen();

/// @brief Prints a char array (string) to "screen" by writing it to VGA buffer.
///
/// @param string The char array (string) to print. To include formatted values, use format_string as declared in string/string.h
/// @param color_bitmap The color the string should be displayed as. For this parameter use VgaColor(bg, fg) where bg/fg are VGA color enums.
void print_color(const char string[], uint8_t color_bitmap);

/// @brief Prints a char array (string) to "screen" by writing it to VGA buffer.
///
/// Does not take a color argument. Uses the default (white text on dark background)
///
/// @param string The char array (string) to print. To include formatted values, use format_string as declared in string/string.
void print(const char string[]);

/// @brief Writes text at a specific location in the "terminal"
///
/// @param row The row in the "terminal" to write to
/// @param column The column in the "terminal" to write to
/// @param string The char array (string) to write. To include formatted values, use format_string as declared in string/string.h
/// @param color_bitmap The color the string should be displayed as. For this parameter use VgaColor(bg, fg) where bg/fg are VGA color enums.
void write_text_at(uint8_t row, uint8_t column, const char string[], uint8_t color_bitmap);