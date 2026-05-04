#include "terminal.h"

static const size_t screen_width = 80;
static const size_t screen_height = 25;

static size_t cursor_row;
static size_t cursor_column; 
static uint8_t color; 
static volatile uint16_t* screen_buffer; 

static inline uint8_t make_color(uint8_t fg, uint8_t bg) {
    return (uint8_t)(fg | (bg << 4));
}

static inline uint16_t make_entry(char c, uint8_t colattr) {
    return (uint16_t)c | ((uint16_t)colattr << 8);
}

void terminal_initialize(void) {
    cursor_row = 0;
    cursor_column = 0;
    color = make_color(15, 0);
    screen_buffer = (volatile uint16_t*)0xB8000; 

    for (size_t y = 0; y < screen_height; y++) {
        for (size_t x = 0; x < screen_width; x++) {
            screen_buffer[y * screen_width + x] = make_entry(' ', color);

        }
        
    }
    
}


void terminal_put_char(char c) {
    if (c == '\n')
    {
        cursor_column = 0;
        cursor_row = (cursor_row + 1) % screen_height;
        return;
    }

    screen_buffer[cursor_row * screen_width + cursor_column] = make_entry(c, color);
    cursor_column++; 

    if (cursor_column >= screen_width) {
        cursor_column = 0;
        cursor_row = (cursor_row + 1) % screen_height;
        return;
    }
}

void terminal_print_string(const char* str) {
    for (size_t i = 0; str[i] != '\0'; i++) {
        terminal_put_char(str[i]);
    }
}