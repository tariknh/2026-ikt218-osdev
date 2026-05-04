#ifndef IRQ_H
#define IRQ_H

// Includes register state structure
#include "isr.h"
// Function type for IRQ handlers
typedef void (*irq_handler_t)(struct registers* regs);
// Installs IRQ handling
void irq_install(void);
// Registers a custom IRQ handler
void irq_install_handler(int irq, irq_handler_t handler);
// Removes a custom IRQ handler
void irq_uninstall_handler(int irq);
// Main IRQ dispatcher
void irq_handler(struct registers* regs);

#endif