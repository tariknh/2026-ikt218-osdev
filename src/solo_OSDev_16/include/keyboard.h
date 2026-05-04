#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <libc/stdint.h>

void keyboard_handle_scancode(uint8_t scancode);

// Getter for monitor 
char get_last_keyboard_key();

#endif