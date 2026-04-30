#include "./vga_text_mode_interface.h"

void VgaTextModeInterfacePrint(struct VgaTextModeInterface* a, char* input, uint8_t attribute) {
    while (*input != 0 && (a->cursor.memory_position < a->cursor.memory_end)) {
        if (*input == '\n') {
            // uint16_t* pos = a->cursor.memory_position;
            int offset = a->cursor.memory_position - a->cursor.memory_start;
            int col = offset % VGA_TERMINAL_WIDTH;

            a->cursor.memory_position += (VGA_TERMINAL_WIDTH - col);
        } else {
            *(a->cursor.memory_position) = (uint16_t)(attribute<<8|*(input));   
            a->cursor.memory_position++;
        }

        input++;
    }
    a->cursor.CalculateRowColFromMemoryPosition(&(a->cursor));
}

void VgaTextModeCursorCalculateRowColFromMemoryPosition(struct VgaTextModeCursor* c){
    uint32_t z = (uint32_t)(c->memory_position - c->memory_start);
    c->row = z / VGA_TERMINAL_WIDTH;
    c->col = z % VGA_TERMINAL_WIDTH;
}

void VgaTextModeCursorSyncHardware(struct VgaTextModeCursor* c) {
    uint16_t position = (uint16_t)(c->memory_position - c->memory_start);

    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(position & 0xFFU));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((position >> 8) & 0xFFU));
}

struct VgaTextModeInterface NewVgaTextModeInterface(){
    struct VgaTextModeInterface i;
    i.Print = VgaTextModeInterfacePrint;
    i.cursor.row = 0;
    i.cursor.col = 0;
    i.cursor.memory_position = (uint16_t*)0xb8000;
    i.cursor.memory_start = (uint16_t*)0xb8000;
    i.cursor.terminal_write_bottom = (uint16_t*)((VGA_TERMINAL_WIDTH * (VGA_TERMINAL_HEIGHT - VGA_MARGIN_BOTTOM_ROWS) * 2) + 0xb8000);
    i.cursor.memory_end = (uint16_t*)((VGA_TERMINAL_WIDTH * VGA_TERMINAL_HEIGHT * 2) + 0xb8000);
    i.cursor.CalculateRowColFromMemoryPosition = VgaTextModeCursorCalculateRowColFromMemoryPosition;
    return i;
}

void copyZeroTerminatedCharArrayToEvenPositionsInCharArray(char* input_array, char* output_array){
    while ( *input_array != 0 ){
        *(output_array++) = *(input_array++);
        output_array++;
    }
}

uint8_t VgaColor(enum standard_vga_text_color bg, enum standard_vga_text_color fg){
    return bg<<4|fg;
}
