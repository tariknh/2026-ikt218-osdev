#include "../vga_text_mode_interface/vga_text_mode_interface.h"

extern struct VgaTextModeInterface main_interface;

void init_vga_interface_for_printing();
void print(const char* string, uint8_t color_bitmap);