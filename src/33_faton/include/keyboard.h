#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <isr.h>

extern volatile char last_key;

void init_keyboard(void);
void keyboard_handler(registers_t* regs, void* context);

#endif