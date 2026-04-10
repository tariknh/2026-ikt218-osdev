#ifndef KERNEL_TERMINAL_H
#define KERNEL_TERMINAL_H

#include <libc/stddef.h>
#include <libc/stdint.h>

enum vga_colour{
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

uint8_t vgaEntryColour(enum vga_colour fg, enum vga_colour bg);
uint16_t vgaEntry(unsigned char uc, uint8_t color);

void terminalEntryAt(char c, uint8_t colour, size_t x, size_t y);
void terminalInitialize(void);
void terminalSetColour(uint8_t colour);
void terminalPutChar(char c);
void terminalWrite(const char* data, size_t size);
void terminalWriteString(const char* data);

#endif