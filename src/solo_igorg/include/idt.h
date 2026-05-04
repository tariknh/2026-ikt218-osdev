#ifndef IDT_H
#define IDT_H

#include <libc/stdint.h>

/*
 * Initializes and loads IDT.
 * IDT allows CPU to locate interrupt service routines 
 * when software interrupts, hardware interrupts, or exceptions occur.
 */
void idt_initialize(void);

/*
 * Registers one interrupt gate in IDT.
 *
 * vector - interrupts vector number
 * handler - address´ of assembly ISR/IRQ stub
 * selector - code segment selector
 * flags - gate flags
 */
void idt_set_gate(uint8_t vector, uint32_t handler, uint16_t selector, uint8_t flags);

#endif
