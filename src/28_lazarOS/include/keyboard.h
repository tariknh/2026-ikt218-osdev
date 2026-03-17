#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <libc/stdint.h>

extern const uint8_t scancode2ascii[256];
extern const uint8_t scancode2ascii_shift[256];
extern const uint8_t scancode2ascii_ctrl[256];

#endif /* KEYBOARD_H */