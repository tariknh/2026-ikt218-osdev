/*
 * irq.h - Hardware interrupt requests (IRQ0..15)
 *
 * Hardware IRQs come in via the two cascaded 8259 PICs. By default they
 * arrive on vectors 0..15, which collide with the CPU exceptions, so we
 * "remap" the PICs to use vectors 32..47 instead.
 *
 * After irq_install():
 *   IRQ 0  -> vector 32  (PIT timer)
 *   IRQ 1  -> vector 33  (PS/2 keyboard)
 *   ...
 *   IRQ 15 -> vector 47
 *
 * The PICs default to "all masked"; use irq_unmask() to enable a specific
 * line, irq_mask() to disable one again.
 */

#pragma once
#include <libc/stdint.h>
#include <isr.h>

/* Remap the PICs and register IRQ0..15 in the IDT. */
void irq_install(void);

/* Register a C-level handler for one IRQ line. */
void irq_register_handler(uint8_t irq_num, isr_handler_t handler);

/* Allow this IRQ line to be delivered to the CPU. */
void irq_unmask(uint8_t irq_num);

/* Block this IRQ line from being delivered. */
void irq_mask(uint8_t irq_num);

/* Called from irq_common in assembly. */
void irq_handler(registers_t* r);

/* The 16 entry points implemented in irq.asm. */
extern void irq0(void);  extern void irq1(void);  extern void irq2(void);  extern void irq3(void);
extern void irq4(void);  extern void irq5(void);  extern void irq6(void);  extern void irq7(void);
extern void irq8(void);  extern void irq9(void);  extern void irq10(void); extern void irq11(void);
extern void irq12(void); extern void irq13(void); extern void irq14(void); extern void irq15(void);
