#pragma once
#include "stdint.h"
struct {
    uint8_t buffer[256];
    uint8_t* head;
} typedef scancode_buffer_t;

struct {
    /** Same as IBM key No. */
    uint16_t keycode;
    /** True if the key was released. False means that the key is pressed down. */
    bool release;
} typedef keycode_t;

struct {
    keycode_t buffer[256];
    keycode_t* head;
} typedef keycode_buffer_t;

void push_scancode_buffer(scancode_buffer_t* buffer, uint8_t scancode);
uint8_t pop_scancode_buffer(scancode_buffer_t* buffer);
scancode_buffer_t create_scancode_buffer();

void push_keycode_buffer(keycode_buffer_t* buffer, keycode_t keycode);
keycode_t pop_keycode_buffer(keycode_buffer_t* buffer);
keycode_buffer_t create_keycode_buffer();

/** Reads through a scancode input buffer until it generates one keycode.
 * Multiple runs of this is needed until the input is empty.
 * \param input Scancode buffer that will be popped from
 * \param output Keycode buffer for output
 */
void scancode2keycode(scancode_buffer_t* input, keycode_buffer_t* output);
keycode_t new_keycode(uint16_t keycode, bool release);
