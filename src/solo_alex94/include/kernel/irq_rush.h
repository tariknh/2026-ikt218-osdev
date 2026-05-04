#ifndef IRQ_RUSH_H
#define IRQ_RUSH_H

#include <libc/stdint.h>

void irq_rush_init(void);
void irq_rush_update(void);
void irq_rush_render(void);
void irq_rush_handle_scancode(uint8_t scancode);

#endif
