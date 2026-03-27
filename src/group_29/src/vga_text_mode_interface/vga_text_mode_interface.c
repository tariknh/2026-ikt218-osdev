#include "./vga_text_mode_interface.h"

void VgaTextModeInterfacePrint(struct VgaTextModeInterface* a, char* input, uint8_t attribute){
    while ( *input != 0 && (a->cursor.memory_position < a->cursor.memory_end)){
        *(a->cursor.memory_position) = (uint16_t)(attribute<<8|*(input));   
        a->cursor.memory_position++;
        input++;
    }
    a->cursor.CalculateRowColFromMemoryPosition(&(a->cursor));
}

void VgaTextModeCursorCalculateRowColFromMemoryPosition(struct VgaTextModeCursor* c){
    uint32_t z = (uint32_t)c->memory_position - (uint32_t)c->memory_start;
    c->row = z / 80;
    c->col = z % 80;
}

struct VgaTextModeInterface NewVgaTextModeInterface(){
    struct VgaTextModeInterface i;
    i.Print = VgaTextModeInterfacePrint;
    i.cursor.row = 0;
    i.cursor.col = 0;
    i.cursor.memory_position = (uint16_t*)0xb8000;
    i.cursor.memory_start = (uint16_t*)0xb8000;
    i.cursor.memory_end = (uint16_t*)((25 * 80 * 2) + 0xb8000);
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