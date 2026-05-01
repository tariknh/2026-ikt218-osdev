#ifndef IRQ_H
#define IRQ_H

#include "isr.h"

/* Install the IRQ handlers and remap the PIC */
void irq_install(void);

/* Allow registering custom C handlers for specific IRQs */
typedef void (*void_function)();
void irq_install_handler(int irq, void (*handler)(registers_t *r));
void irq_uninstall_handler(int irq);

#endif /* IRQ_H */
