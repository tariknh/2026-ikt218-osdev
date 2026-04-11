#include "printing.h"

struct VgaTextModeInterface main_interface;

void init_vga_interface_for_printing() {
    main_interface = NewVgaTextModeInterface();
}

void print(const char* string, uint8_t color_bitmap) {
    while (*string != 0 && (main_interface.cursor.memory_position < main_interface.cursor.memory_end)) {
        if (*string == '\n') {
            // uint16_t* pos = main_interface.cursor.memory_position;
            int offset = main_interface.cursor.memory_position - main_interface.cursor.memory_start;
            int col = offset % VGA_TERMINAL_WIDTH;

            main_interface.cursor.memory_position += (VGA_TERMINAL_WIDTH - col);
        } else {
            *(main_interface.cursor.memory_position) = (uint16_t)(color_bitmap<<8|*(string));   
            main_interface.cursor.memory_position++;
        }

        string++;
    }
    main_interface.cursor.CalculateRowColFromMemoryPosition(&(main_interface.cursor));
}