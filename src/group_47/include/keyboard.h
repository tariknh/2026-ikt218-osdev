#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "isr.h"

void keyboard_handler(registers_t *r);
char* get_keyboard_buffer(void);
int is_key_pressed(void);
void clear_key_pressed(void);
char get_ascii_key(void);

#endif