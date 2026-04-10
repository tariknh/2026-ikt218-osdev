#include "idt.h"
#include "terminal.h"

// We'll need a way to talk to I/O ports (defined in assembly or a helper)
extern void outb(uint16_t port, uint8_t val);

// Function pointers for IRQ handlers
void *irq_routines[16] = { 0 };

// External assembly stubs for IRQ 0-15
extern void irq0();
extern void irq1();
extern void irq2();
extern void irq3();
extern void irq4();
extern void irq5();
extern void irq6();
extern void irq7();
extern void irq8();
extern void irq9();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();

void irq_install_handler(int irq, void (*handler)(struct registers *r)) {
    irq_routines[irq] = handler;
}

void irq_remap() {
    // Send initialization commands to Master and Slave PIC
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    // Map Master PIC to IDT vectors 32-39
    outb(0x21, 0x20);
    // Map Slave PIC to IDT vectors 40-47
    outb(0xA1, 0x28);
    // Setup cascading and environment (standard protocol)
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    // Unmask interrupts
    outb(0x21, 0x0);
    outb(0xA1, 0x0);
}

void irq_install() {
    irq_remap();

    // Map the IRQs to our IDT
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

void irq_handler(struct registers *r) {
    if (r->int_no >= 40) {
        outb(0xA0, 0x20);
    }
    outb(0x20, 0x20);

    void (*handler)(struct registers *r);
    handler = irq_routines[r->int_no - 32];
    if (handler) {
        handler(r);
    }
}