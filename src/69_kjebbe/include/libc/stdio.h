#pragma once

#include "stdbool.h"
#include "stddef.h"
#include "stdint.h"
// Solid colours for the Norwegian flag
#define VGA_RED 0x44
#define VGA_WHITE 0x77
#define VGA_BLUE 0x11

// Foreground colours on black background
#define VGA_BLUE_ON_BLACK 0x01
#define VGA_GREEN_ON_BLACK 0x02
#define VGA_CYAN_ON_BLACK 0x03
#define VGA_RED_ON_BLACK 0x04
#define VGA_MAGENTA_ON_BLACK 0x05
#define VGA_BROWN_ON_BLACK 0x06
#define VGA_GREY_ON_BLACK 0x07
#define VGA_DARK_GREY_ON_BLACK 0x08
#define VGA_LIGHT_BLUE_ON_BLACK 0x09
#define VGA_LIGHT_GREEN_ON_BLACK 0x0A
#define VGA_LIGHT_CYAN_ON_BLACK 0x0B
#define VGA_LIGHT_RED_ON_BLACK 0x0C
#define VGA_LIGHT_MAGENTA_ON_BLACK 0x0D
#define VGA_YELLOW_ON_BLACK 0x0E
#define VGA_WHITE_ON_BLACK 0x0F

int printf(const char *__restrict__ format, ...);
int printf_color(const char *__restrict__ format, uint8_t terminal_color, ...);
void clearTerminal();
void terminal_set_char(int x, int y, char c, uint8_t terminal_color);
