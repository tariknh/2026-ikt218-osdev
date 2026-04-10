#include <kernel/terminal.h>

const size_t VGA_HEIGHT = 25;
const size_t VGA_WIDTH = 80;

static size_t terminal_row;
static size_t terminal_column;

static uint8_t terminal_colour;
static uint16_t* terminal_buffer = (uint16_t*) 0xB8000;

// Builds the colour byte
uint8_t vgaEntryColour(enum vga_colour fg, enum vga_colour bg) {
    return fg | bg << 4;
}

// Builds the full cell
uint16_t vgaEntry(unsigned char uc, uint8_t color) {
    return (uint16_t) uc | (uint16_t) color << 8;
}

void terminalEntryAt(char c, uint8_t colour, size_t x, size_t y){
    size_t index = y * VGA_WIDTH + x;
    terminal_buffer[index] = vgaEntry(c, colour);
}

void terminalInitialize(void) {
    terminal_row = 0;
    terminal_column = 0;
    terminal_colour = vgaEntryColour(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    terminal_buffer = (uint16_t*) 0xB8000;

    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            terminalEntryAt(' ', terminal_colour, x, y);
        }
    }
}

void terminalSetColour(uint8_t colour) {
    terminal_colour = colour;
}

void terminalPutChar(char c) {
    if (c == '\n') {
        terminal_column = 0;
        terminal_row++;
        return;
    }
    terminalEntryAt(c, terminal_colour,terminal_column, terminal_row);
    terminal_column++;

    if (terminal_column == VGA_WIDTH) {
        terminal_column = 0;
        terminal_row++;
    }

    if (terminal_row == VGA_HEIGHT) {
        terminal_row = 0;
    }
}

void terminalWrite(const char* data, size_t size) {
    for (size_t i = 0; i < size; i++) {
        terminalPutChar(data[i]);
    }
}

void terminalWriteString(const char* data) {
    size_t len = 0;

    while (data[len] != '\0') {
        len++;
    }

    terminalWrite(data, len);
}

