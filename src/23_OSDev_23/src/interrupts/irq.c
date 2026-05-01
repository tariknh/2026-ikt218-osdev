#include "irq.h"
#include "idt.h"
#include "ports.h"
#include <stddef.h>

/* Array of pointers to handle custom IRQ handlers */
void *irq_routines[16] = {
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
};

/* Install a handler for a given IRQ */
void irq_install_handler(int irq, void (*handler)(registers_t *r)) {
    irq_routines[irq] = handler;
}

/* Uninstall a handler */
void irq_uninstall_handler(int irq) {
    irq_routines[irq] = NULL;
}

/* IRQ declarations from our assembly file */
extern void irq0();  extern void irq1();  extern void irq2();  extern void irq3();
extern void irq4();  extern void irq5();  extern void irq6();  extern void irq7();
extern void irq8();  extern void irq9();  extern void irq10(); extern void irq11();
extern void irq12(); extern void irq13(); extern void irq14(); extern void irq15();

/* Remap the PIC to route IRQs to IDT entries 32-47 instead of 0-15 */
void irq_remap(void) {
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    outb(0x21, 0x20); // Master PIC vector offset to 32
    outb(0xA1, 0x28); // Slave PIC vector offset to 40
    outb(0x21, 0x04); // Master PIC has slave at IRQ2
    outb(0xA1, 0x02); // Slave PIC cascade identity
    outb(0x21, 0x01); // 8086/88 mode
    outb(0xA1, 0x01); // 8086/88 mode
    outb(0x21, 0x00); // Unmask all IRQs
    outb(0xA1, 0x00); // Unmask all IRQs
}

void irq_install(void) {
    irq_remap();
    idt_set_gate(32, (uint32_t)irq0, 0x08, 0x8E);
    idt_set_gate(33, (uint32_t)irq1, 0x08, 0x8E);
    idt_set_gate(34, (uint32_t)irq2, 0x08, 0x8E);
    idt_set_gate(35, (uint32_t)irq3, 0x08, 0x8E);
    idt_set_gate(36, (uint32_t)irq4, 0x08, 0x8E);
    idt_set_gate(37, (uint32_t)irq5, 0x08, 0x8E);
    idt_set_gate(38, (uint32_t)irq6, 0x08, 0x8E);
    idt_set_gate(39, (uint32_t)irq7, 0x08, 0x8E);
    idt_set_gate(40, (uint32_t)irq8, 0x08, 0x8E);
    idt_set_gate(41, (uint32_t)irq9, 0x08, 0x8E);
    idt_set_gate(42, (uint32_t)irq10, 0x08, 0x8E);
    idt_set_gate(43, (uint32_t)irq11, 0x08, 0x8E);
    idt_set_gate(44, (uint32_t)irq12, 0x08, 0x8E);
    idt_set_gate(45, (uint32_t)irq13, 0x08, 0x8E);
    idt_set_gate(46, (uint32_t)irq14, 0x08, 0x8E);
    idt_set_gate(47, (uint32_t)irq15, 0x08, 0x8E);
}

void irq_handler(registers_t *r) {
    /* This is a blank function pointer */
    void (*handler)(registers_t *r);

    /* Find out if we have a custom handler to run for this IRQ, and then finally run it */
    handler = irq_routines[r->int_no - 32];
    if (handler) {
        handler(r);
    }

    /* If the IDT entry that was invoked was greater than 40
    *  (meaning IRQ8 - 15), then we need to send an EOI to
    *  the slave controller */
    if (r->int_no >= 40) {
        outb(0xA0, 0x20);
    }
    
    /* In either case, we need to send an EOI to the master
    *  interrupt controller too */
    outb(0x20, 0x20);
}
