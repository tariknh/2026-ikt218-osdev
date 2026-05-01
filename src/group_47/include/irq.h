#ifndef IRQ_H
#define IRQ_H

#include "libc/stdint.h"
#include "isr.h"

// Prototypes for the functions in irq.c
void irq_install();
void irq_remap();
void irq_handler(registers_t *r);

// Helper to install specific hardware handlers
void irq_install_handler(int irq, void (*handler)(registers_t *r));
void irq_uninstall_handler(int irq);

#endif