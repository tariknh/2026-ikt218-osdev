#pragma once
#include "isr.h"

typedef void (*irq_handler_t)(registers_t* regs);

void irq_init(void);
void irq_register_handler(int irq, irq_handler_t handler);
