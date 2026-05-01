#ifndef IRQ_H

// for in-depth explanations of the following variables look to
// https://wiki.osdev.org/8259_PIC
#define IRQ_H
#define PIC1 0x20            // address to the master PIC
#define PIC2 0xA0            // address to the slave PIC
#define PIC1_COMMAND PIC1    // address to the MASTER PIC command port
#define PIC1_DATA (PIC1 + 1) // address to the MASTER PIC data port
#define PIC2_COMMAND PIC2    // address to the SLAVE PIC command port
#define PIC2_DATA (PIC2 + 1) // address to the SLAVE PIC DATA port

#define ICW1_INIT 0x10 // Initialization sent to command port on startup
#define ICW1_ICW4                                                              \
  0x01 // Used on command port indicates that ICW4 will be present

#define ICW4_8086 0x01 // 8086 CPU mode
#define CASCADE_IRQ 2

#include "isr.h" // reuse registers_t

// More info in the irq.c file
void irq_init(void);

// More info in the irq.c file
void irq_handler(registers_t *regs);

// More info in the irq.c file
void irq_register_handler(int irq, void (*handler)(registers_t *));

// Makes entrypoints to various interrupt request defined in the irq.asm file
// available in C.
extern void irq0(void);
extern void irq1(void);
extern void irq2(void);
extern void irq3(void);
extern void irq4(void);
extern void irq5(void);
extern void irq6(void);
extern void irq7(void);
extern void irq8(void);
extern void irq9(void);
extern void irq10(void);
extern void irq11(void);
extern void irq12(void);
extern void irq13(void);
extern void irq14(void);
extern void irq15(void);

#endif
