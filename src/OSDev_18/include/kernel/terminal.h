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

uint8_t VgaEntryColour(enum vga_colour fg, enum vga_colour bg);
uint16_t VgaEntry(unsigned char uc, uint8_t color);

void TerminalClear(void);
void TerminalEntryAt(char c, uint8_t colour, size_t x, size_t y);
void TerminalInitialize(void);
void TerminalSetColour(uint8_t colour);
void TerminalUpdateCursor(void);
void TerminalPutChar(char c);
void TerminalWrite(const char* data, size_t size);
void TerminalWriteString(const char* data);
void TerminalWriteUInt(uint32_t num);
void TerminalWriteHex(uint32_t memory);
char TerminalGetChar(void);
int TerminalGetUInt(uint32_t *number);

#endif