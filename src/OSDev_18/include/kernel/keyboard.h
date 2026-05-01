#ifndef KERNEL_KEYBOARD_H
#define KERNEL_KEYBOARD_H

#include <kernel/interrupt.h>

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_BUFFER_SIZE 256

char GetLastKeyPressed(void);

void KeyboardHandler(struct Registers* registers);

#endif