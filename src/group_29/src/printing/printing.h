#pragma once

#include "../vga_text_mode_interface/vga_text_mode_interface.h"
#include "../memory/memory_macros.h"
#include "libc/stdint.h"

// #define TEMP VGA_TERMINAL_WIDTH * sizeof(uint16_t) * 2

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


/// @brief Clears the VGA text screen using the default color.
///
/// Fills the entire screen with blank spaces using a default
/// foreground/background color combination. The cursor is reset
/// to the beginning of the writable area (top margin).
///
/// @note This is a convenience wrapper around fill_screen().
void clear_screen();


/// @brief Fills the entire VGA text screen with blank characters.
///
/// Writes a space character (' ') to every cell in the VGA text buffer,
/// using the provided color attribute. After clearing, the cursor is
/// repositioned to the start of the writable area (top margin), and
/// synchronized with the hardware cursor.
///
/// @param color_bitmap VGA color attribute (background << 4 | foreground).
void fill_screen(uint8_t color_bitmap);