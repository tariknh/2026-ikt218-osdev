#include "terminal.h"
#include <libc/stdint.h>

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_COLOR_WHITE_ON_BLACK 0x0F

static uint16_t *video_memory = (uint16_t *)0xB8000;
static int terminal_column = 0;
static int terminal_row = 0;

static void terminal_newline(void)
{
    terminal_column = 0;
    terminal_row++;

    if (terminal_row >= VGA_HEIGHT) {
        terminal_row = 0;
    }
}

void terminal_clear(void)
{
    for (int row = 0; row < VGA_HEIGHT; row++) {
        for (int column = 0; column < VGA_WIDTH; column++) {
            int index = row * VGA_WIDTH + column;
            video_memory[index] = (uint16_t)' ' | (uint16_t)(VGA_COLOR_WHITE_ON_BLACK << 8);
        }
    }

    terminal_column = 0;
    terminal_row = 0;
}

void terminal_putchar_at(char character, uint8_t color, int column, int row)
{
    if (column < 0 || column >= VGA_WIDTH || row < 0 || row >= VGA_HEIGHT) {
        return;
    }

    int index = row * VGA_WIDTH + column;
    video_memory[index] = (uint16_t)character | (uint16_t)(color << 8);
}

void terminal_write(const char *text)
{
    int i = 0;

    while (text[i] != '\0') {
        if (text[i] == '\n') {
            terminal_newline();
            i++;
            continue;
        }

        terminal_putchar_at(text[i], VGA_COLOR_WHITE_ON_BLACK, terminal_column, terminal_row);

        terminal_column++;

        if (terminal_column >= VGA_WIDTH) {
            terminal_newline();
        }

        i++;
    }
}

void terminal_write_dec(uint32_t value)
{
    char buffer[11];
    int index = 0;

    if (value == 0) {
        terminal_write("0");
        return;
    }

    while (value > 0 && index < 10) {
        buffer[index] = (char)('0' + (value % 10));
        value /= 10;
        index++;
    }

    while (index > 0) {
        index--;
        char text[2];
        text[0] = buffer[index];
        text[1] = '\0';
        terminal_write(text);
    }
}

void terminal_write_hex(uint32_t value)
{
    const char *hex_digits = "0123456789ABCDEF";

    terminal_write("0x");

    for (int shift = 28; shift >= 0; shift -= 4) {
        uint8_t digit = (uint8_t)((value >> shift) & 0xF);

        char text[2];
        text[0] = hex_digits[digit];
        text[1] = '\0';

        terminal_write(text);
    }
}
