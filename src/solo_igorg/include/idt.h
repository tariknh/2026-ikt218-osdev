#ifndef IDT_H
#define IDT_H

#include <libc/stdint.h>

/*
 * Initializes and loads IDT.
 * IDT allows CPU to locate interrupt service routines 
 * when software interrupts, hardware interrupts, or exceptions occur.
 */
void idt_initialize(void);

#endif
