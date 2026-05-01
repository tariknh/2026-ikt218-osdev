#ifndef UIAOS_KERNEL_PIANO_H
#define UIAOS_KERNEL_PIANO_H

#include <stdint.h>

void piano_init(void);
void piano_handle_scancode(uint8_t scancode);

#endif
