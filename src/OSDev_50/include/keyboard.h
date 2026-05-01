#ifndef KEYBOARD_H
#define KEYBOARD_H
#pragma once
#include <libc/stdint.h> 

uint8_t keyboard_last_scancode(void);
void keyboard_init(void);

char keyboard_last_key(void);
uint32_t keyboard_irq_count(void);

#endif
