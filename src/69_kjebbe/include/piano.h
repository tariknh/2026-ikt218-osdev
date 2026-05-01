#ifndef PIANO_H
#define PIANO_H

#include "libc/stdint.h"

void piano_keyboard_handler(uint8_t scancode);
extern int piano_played_key;

#endif
