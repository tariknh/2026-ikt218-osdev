#pragma once

#include "stdint.h"
#include "../io/io.h"

#define VGA_TERMINAL_WIDTH 80
#define VGA_TERMINAL_HEIGHT 25

#define VGA_MARGIN_TOP_ROWS 1
#define VGA_MARGIN_BOTTOM_ROWS 1

/**
 * \enum Gives acces to standard VGA text mode colors.
 * 
 * \note In MDA mode you can set underlines instead of color. [How to switch to MDA mode?]
 * \see https://en.wikipedia.org/wiki/VGA_text_mode#Underline
 * 
 * \note Attribiute bit 7 can be used to make the text blink.
 * In this case setting the background color to an high intensity color (>7)
 * will make the text blink with the corresponding low intensity background color.
 * \see https://en.wikipedia.org/wiki/VGA_text_mode#Text_buffer
 * 
 * \note VGA text mode supports multiple fonts. And two of them can be used at the same time.
 * But in that case attribute bit 3 will change the font for the corresponding cell.
 * So high intensity foreground colors will change the font if two are loaded.
 * This should not be a problem if both font pointers point to the same font.
 * On some VGA cards both the color and font will be changed at the same time.
 * If you want two fonts with the same colors you can manually change the color palette.
 * \see https://en.wikipedia.org/wiki/VGA_text_mode#Fonts
 * 
 * \note The color palette can be changed using VGA commands. [How?]
 * \see https://wiki.osdev.org/Text_UI#Colours
 */
enum standard_vga_text_color{
    vga_black = 0,
    vga_blue = 1,
    vga_green = 2,
    vga_cyan = 3,
    vga_red = 4,
    vga_magenta = 5,
    vga_brown = 6,
    vga_light_gray = 7,
    vga_dark_gray = 8,
    vga_light_blue = 9,
    vga_light_green = 10,
    vga_light_cyan = 11,
    vga_light_red = 12,
    vga_light_magenta = 13,
    vga_yellow = 14,
    vga_white = 15
};

/** \brief Puts together two values from standard_vga_text_color into a single uint8_t 
 *  \todo Switch to an oop system that handles underlaines and multiple fonts
*/
uint8_t VgaColor(enum standard_vga_text_color bg, enum standard_vga_text_color fg);

/// @brief Copies every element of one array to every even position in the output array until it encounters a 0.
///
/// input_array = [a,b,0] and input_array = [c,d,e,f] wil make output_array become [a,d,b,f]
/// This function is mostly to help with VGA text mode.
/// @param input_array A pointer to the start of the zero-terminated input array.
/// @param output_array A pointer to the start of the output array
/// \warning The input array MUST be zero-terminated.
/// \note Does not change the odd values in the output array.
/// \deprecated Use VgaTextModeInterface.Print instead
void copyZeroTerminatedCharArrayToEvenPositionsInCharArray(char* input_array, char* output_array);

//VGA resolution 80 x 25
struct VgaTextModeCursor{
    uint8_t row;
    uint8_t col;
    uint16_t* memory_position;
    uint16_t* memory_start;
    uint16_t *terminal_write_bottom;
    uint16_t *memory_end;
    void (*CalculateRowColFromMemoryPosition)(struct VgaTextModeCursor*);
};

/** Use NewVgaTextModeInterface() to create an instance of this struct */
struct VgaTextModeInterface{
    struct VgaTextModeCursor cursor;
    /** \brief Print a 0 terminated char array to the screen
     * \param attribute Attribute to set for the text. Applies to the entire string. See enum standard_vga_text_color
     * \param input An pointer to an array of ASCII characters, terminated by 0.
    */
    void (*Print)(struct VgaTextModeInterface*, char* input, uint8_t attribute);
};

/** \brief See VgaTextModeInterface.Print() */
void VgaTextModeInterfacePrint(struct VgaTextModeInterface* a, char* input, uint8_t attribute);
/** \brief See VgaTextModeCursor.CalculateRowColFromMemoryPosition()
 * \todo Check that this works correctly
 */
void VgaTextModeCursorCalculateRowColFromMemoryPosition(struct VgaTextModeCursor* c);
/** \brief Updates the VGA hardware cursor to the current text cell. */
void VgaTextModeCursorSyncHardware(struct VgaTextModeCursor* c);
/** \brief Initialises a VgaTextModeInterface and returns it. */
struct VgaTextModeInterface NewVgaTextModeInterface();
