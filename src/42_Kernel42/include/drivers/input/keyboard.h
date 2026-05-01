#pragma once
#include <stdbool.h>
#include <stdint.h>


/**
 * Initializes the keyboard, with its interrupt handler
 */
void init_keyboard();

/**
 * Set hardware keyboard to emit keycodes from set 2. See https://wiki.osdev.org/PS/2_Keyboard.
 */
void keyboard_set_scancode_set2();

/**
 * Decode all cached scancodes into the pressed keys and special keys and push them to respective
 * buffers.
 */
void decode_keyboard();


/**
 * Pop a key off the keybuffer
 * @param out key popped
 * @return false if keybuffer is empty
 */
bool pop_key(uint8_t* out);


/**
 * Check if the special keybuffer has any elements.
 * A special key, are keys such as the function keys, arrow keys, home, end, page up/down.
 * @return true if it has 1 or more elements
 */
bool has_special_key();

/**
 * Pop a special key off the keybuffer
 * @return false if keybuffer is empty
 */
bool pop_special_key(uint8_t* out);

/**
 * Check if the ascii keybuffer has any elements.
 * @return true if it has 1 or more elements
 */
bool keyboard_has_key(void);
