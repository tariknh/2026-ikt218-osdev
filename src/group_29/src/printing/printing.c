#include "printing.h"

struct VgaTextModeInterface main_interface;

void init_vga_interface_for_printing() {
    main_interface = NewVgaTextModeInterface();
    main_interface.cursor.CalculateRowColFromMemoryPosition(&(main_interface.cursor));
    VgaTextModeCursorSyncHardware(&(main_interface.cursor));
}


void scroll_screen() {
    uint16_t* screen_start = main_interface.cursor.memory_start;

    // Calculate how many rows need to be scrolled
    int rows_to_scroll = VGA_TERMINAL_HEIGHT - VGA_MARGIN_BOTTOM_ROWS - VGA_MARGIN_TOP_ROWS;  // 5 bottom rows are reserved, adjust if necessary

    // Move the content up by one row, starting from row MARGIN_TOP_ROWS
    for (int row = VGA_MARGIN_TOP_ROWS; row < VGA_TERMINAL_HEIGHT - VGA_MARGIN_BOTTOM_ROWS - 1; row++) {
        for (int col = 0; col < VGA_TERMINAL_WIDTH; col++) {
            uint16_t* current_pos = screen_start + (row * VGA_TERMINAL_WIDTH + col);
            uint16_t* next_row_pos = screen_start + ((row + 1) * VGA_TERMINAL_WIDTH + col);
            *current_pos = *next_row_pos;  // Move each row's content up
        }
    }

    // Clear the last line (the last usable row before the bottom margin)
    for (int col = 0; col < VGA_TERMINAL_WIDTH; col++) {
        uint16_t* last_line = screen_start + ((VGA_TERMINAL_HEIGHT - VGA_MARGIN_BOTTOM_ROWS - 1) * VGA_TERMINAL_WIDTH + col);
        *last_line = 0;  // Clear the last row before the bottom margin
    }

    // Reset the cursor to the new last row before the unused bottom margin
    main_interface.cursor.memory_position = screen_start + ((VGA_TERMINAL_HEIGHT - VGA_MARGIN_BOTTOM_ROWS - 1) * VGA_TERMINAL_WIDTH);
    main_interface.cursor.CalculateRowColFromMemoryPosition(&(main_interface.cursor));
    VgaTextModeCursorSyncHardware(&(main_interface.cursor));
}

void print_color(const char string[], uint8_t color_bitmap) {
    // Loop over each character in the string
    while (*string != 0) {
        // Check if the cursor has reached the end of the screen
        if (main_interface.cursor.memory_position >= main_interface.cursor.terminal_write_bottom) {
            // Scroll the screen if we've reached the bottom
            scroll_screen();
        }

        // If we encounter a newline character, move the cursor to the next line
        if (*string == '\n') {
            // Move the cursor to the next row
            int offset = main_interface.cursor.memory_position - main_interface.cursor.memory_start;
            int col = offset % VGA_TERMINAL_WIDTH;

            // Move the cursor to the start of the next row
            main_interface.cursor.memory_position += (VGA_TERMINAL_WIDTH - col);
        } else {
            // Write the character to the screen at the current cursor position
            *(main_interface.cursor.memory_position) = (uint16_t)(color_bitmap << 8 | *(string));
            main_interface.cursor.memory_position++;
        }

        string++;  // Move to the next character in the string
    }

    // After printing, update the cursor position
    main_interface.cursor.CalculateRowColFromMemoryPosition(&(main_interface.cursor));
    VgaTextModeCursorSyncHardware(&(main_interface.cursor));
}

void print(const char string[]) {
    print_color(string, VgaColor(vga_black, vga_white));
}

void write_text_at(uint8_t row, uint8_t column, const char string[], uint8_t color_bitmap) {
    uint16_t* cursor = VGA_MEMORY + (row * VGA_WIDTH) + column;

    while (*string != '\0' && column < VGA_WIDTH) {
        *cursor = (uint16_t)((color_bitmap << 8) | (uint8_t)*string);
        ++cursor;
        ++column;
        ++string;
    }
}
