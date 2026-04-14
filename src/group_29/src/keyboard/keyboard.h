#pragma once

#include "libc/stdbool.h"
#include "libc/stdint.h"

#define KEYBOARD_BUFFER_SIZE 256

typedef struct {
    uint8_t buffer[KEYBOARD_BUFFER_SIZE];
    uint16_t head;
    uint16_t tail;
    uint16_t count;
} scancode_buffer_t;

typedef struct {
    uint16_t keycode;
    bool release;
} keycode_t;

typedef struct {
    keycode_t buffer[KEYBOARD_BUFFER_SIZE];
    uint16_t head;
    uint16_t tail;
    uint16_t count;
} keycode_buffer_t;

void push_scancode_buffer(scancode_buffer_t* buffer, uint8_t scancode);
uint8_t pop_scancode_buffer(scancode_buffer_t* buffer);
scancode_buffer_t create_scancode_buffer(void);

void push_keycode_buffer(keycode_buffer_t* buffer, keycode_t keycode);
keycode_t pop_keycode_buffer(keycode_buffer_t* buffer);
keycode_buffer_t create_keycode_buffer(void);

void scancode2keycode(scancode_buffer_t* input, keycode_buffer_t* output);
keycode_t new_keycode(uint16_t keycode, bool release);

void init_keyboard(void);
void keyboard_handle_scancode(uint8_t scancode);
bool keyboard_has_char(void);
char keyboard_pop_char(void);
