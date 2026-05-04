#ifndef IRQ_H
#define IRQ_H

#include <libc/stdint.h>

//remaps the PIC and registers handlers for IRQ0-15
void init_irq(void);

//called by asm stub when a hardware interrupt happens
void irq_handler(uint32_t irq_number);

//asm stubs for IRQ0-15, declared here so idt.c can register addresses
void irq0(void);
void irq1(void);
void irq2(void);
void irq3(void);
void irq4(void);
void irq5(void);
void irq6(void);
void irq7(void);
void irq8(void);
void irq9(void);
void irq10(void);
void irq11(void);
void irq12(void);
void irq13(void);
void irq14(void);
void irq15(void);

#endif