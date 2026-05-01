#include <kernel/gdt.h>
#include <kernel/terminal.h>
#include <kernel/io.h>
#include <kernel/keyboard.h>
#include <kernel/pit.h>
#include <libc/limits.h>

const size_t VGA_HEIGHT = 25;
const size_t VGA_WIDTH = 80;

static size_t terminal_row;
static size_t terminal_column;

static uint8_t terminal_colour;
static uint16_t* terminal_buffer = (uint16_t*) 0xB8000;

// Builds the colour byte
uint8_t VgaEntryColour(enum vga_colour fg, enum vga_colour bg) {
    return fg | bg << 4;
}

// Builds the full cell
uint16_t VgaEntry(unsigned char uc, uint8_t color) {
    return (uint16_t) uc | (uint16_t) color << 8;
}

void TerminalClear(void) {
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            TerminalEntryAt(' ', terminal_colour, x, y);
        }
    }

    terminal_row = 0;
    terminal_column = 0;
    TerminalUpdateCursor();
}

void TerminalEntryAt(char c, uint8_t colour, size_t x, size_t y){
    size_t index = y * VGA_WIDTH + x;
    terminal_buffer[index] = VgaEntry(c, colour);
}

void TerminalInitialize(void) {
    terminal_row = 0;
    terminal_column = 0;
    terminal_colour = VgaEntryColour(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    terminal_buffer = (uint16_t*) 0xB8000;

    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            TerminalEntryAt(' ', terminal_colour, x, y);
        }
    }
}

void TerminalSetColour(uint8_t colour) {
    terminal_colour = colour;
}

void TerminalUpdateCursor(void) {
    uint16_t pos = (uint16_t)(terminal_row * VGA_WIDTH + terminal_column);

    OutPortByte(0x3D4, 0x0F);
    OutPortByte(0x3D5, (uint8_t)(pos & 0xFF));

    OutPortByte(0x3D4, 0x0E);
    OutPortByte(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

void TerminalPutChar(char c) {
    if (c == '\n') {
        terminal_column = 0;
        terminal_row++;
        TerminalUpdateCursor();
        return;
    }
    TerminalEntryAt(c, terminal_colour,terminal_column, terminal_row);
    terminal_column++;

    if (terminal_column == VGA_WIDTH) {
        terminal_column = 0;
        terminal_row++;
    }

    if (terminal_row == VGA_HEIGHT) {
        terminal_row = 0;
    }

    TerminalUpdateCursor();
}

void TerminalWrite(const char* data, size_t size) {
    for (size_t i = 0; i < size; i++) {
        TerminalPutChar(data[i]);
    }
}

void TerminalWriteString(const char* data) {
    size_t len = 0;

    while (data[len] != '\0') {
        len++;
    }

    TerminalWrite(data, len);
}

void TerminalWriteUInt(uint32_t num) {
    if (num == 0) {
        TerminalPutChar('0');
        return;
    }

    char buffer[10];
    size_t i = 0;

    while (num > 0) {
        buffer[i++] = '0' + (num % 10);
        num /= 10;
    }

    for (size_t j = 0; j < i / 2; j++) {
        char temp = buffer[j];
        buffer[j] = buffer[i - j - 1];
        buffer[i - j - 1] = temp;
    }

    for (size_t j = 0; j < i; j++) {
        TerminalPutChar(buffer[j]);
    }
}

void TerminalWriteHex(uint32_t num) {
    TerminalWriteString("0x");

    char hexChars[] = "0123456789ABCDEF";

    for (int i = 28; i >= 0; i -= 4) {
        uint8_t digit = (num >> i) & 0xF;
        TerminalPutChar(hexChars[digit]);
    }
}

char TerminalGetChar(void) {
    while (1) {
        char key = GetLastKeyPressed();

        if (key != 0) {
            return key;
        }

        SleepInterrupt(100);
    }
}

int TerminalGetUInt(uint32_t *number) {
    uint32_t num = 0;
    int nextIndex = 0;
    char key = 0;
    int error = 0;

    do {
        key = GetLastKeyPressed();

        if (key >= '0' && key <= '9') {
            uint32_t digit = key - '0';

            if (num <= (UINT32_MAX - digit) / 10) {
                num = num * 10 + digit;
                nextIndex++;
            } else {
                error = 1;
            }
        } else if (key != 0 && key != '\n') {
            error = 1;
        }

        SleepInterrupt(100);
    } while (key != 10 && nextIndex < 10);

    if (error || nextIndex == 0) return 0;

    *number = num;
    return 1;
}
