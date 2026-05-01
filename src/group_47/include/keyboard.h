#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "isr.h"

void keyboard_handler(registers_t *r);
char* get_keyboard_buffer(void);

#endif