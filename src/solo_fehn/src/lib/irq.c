/*
 * irq.c - Hardware interrupt support
 *
 * The 8259 Programmable Interrupt Controllers (one master, one slave) hand
 * IRQ lines from devices to the CPU as interrupt vectors. By default they
 * deliver IRQ0..IRQ15 on vectors 0..15, which collide with the CPU
 * exceptions we just installed in the IDT.  To avoid that we "remap" them
 * to vectors 32..47 by reprogramming the PICs at startup.
 *
 * Every IRQ handler must end by sending an "End Of Interrupt" (EOI) byte
 * back to the PIC, otherwise the line stays masked and no further IRQs of
 * that type are delivered.
 *
 * After remapping we set the interrupt mask to "all masked" (0xFF on each
 * PIC) so that no spurious IRQ is delivered until the kernel has actually
 * registered a handler for that line. Drivers call irq_unmask() to enable
 * the lines they care about.
 */

#include <irq.h>
#include <idt.h>
#include <io.h>
#include <libc/stdint.h>

#define PIC1_CMD    0x20
#define PIC1_DATA   0x21
#define PIC2_CMD    0xA0
#define PIC2_DATA   0xA1
#define PIC_EOI     0x20

#define ICW1_INIT   0x10
#define ICW1_ICW4   0x01
#define ICW4_8086   0x01

static isr_handler_t irq_handlers[16];

static void pic_remap(int offset_master, int offset_slave) {
    /* ICW1: start initialisation sequence (cascade mode, edge-triggered). */
    outb(PIC1_CMD, ICW1_INIT | ICW1_ICW4); io_wait();
    outb(PIC2_CMD, ICW1_INIT | ICW1_ICW4); io_wait();

    /* ICW2: vector offset for IRQ0 / IRQ8 */
    outb(PIC1_DATA, (uint8_t)offset_master); io_wait();
    outb(PIC2_DATA, (uint8_t)offset_slave);  io_wait();

    /* ICW3: tell the master that there's a slave on IRQ2, and tell the
       slave its cascade identity is 2. */
    outb(PIC1_DATA, 0x04); io_wait();
    outb(PIC2_DATA, 0x02); io_wait();

    /* ICW4: 8086/88 mode (the only mode we want). */
    outb(PIC1_DATA, ICW4_8086); io_wait();
    outb(PIC2_DATA, ICW4_8086); io_wait();

    /* Mask everything by default. Drivers will call irq_unmask() on the
       lines they actually want. */
    outb(PIC1_DATA, 0xFF);
    outb(PIC2_DATA, 0xFF);
}

void irq_install(void) {
    pic_remap(0x20, 0x28);   /* IRQ0..7 -> 32..39, IRQ8..15 -> 40..47 */

    idt_set_gate(32, (uint32_t)irq0,  0x08, 0x8E);
    idt_set_gate(33, (uint32_t)irq1,  0x08, 0x8E);
    idt_set_gate(34, (uint32_t)irq2,  0x08, 0x8E);
    idt_set_gate(35, (uint32_t)irq3,  0x08, 0x8E);
    idt_set_gate(36, (uint32_t)irq4,  0x08, 0x8E);
    idt_set_gate(37, (uint32_t)irq5,  0x08, 0x8E);
    idt_set_gate(38, (uint32_t)irq6,  0x08, 0x8E);
    idt_set_gate(39, (uint32_t)irq7,  0x08, 0x8E);
    idt_set_gate(40, (uint32_t)irq8,  0x08, 0x8E);
    idt_set_gate(41, (uint32_t)irq9,  0x08, 0x8E);
    idt_set_gate(42, (uint32_t)irq10, 0x08, 0x8E);
    idt_set_gate(43, (uint32_t)irq11, 0x08, 0x8E);
    idt_set_gate(44, (uint32_t)irq12, 0x08, 0x8E);
    idt_set_gate(45, (uint32_t)irq13, 0x08, 0x8E);
    idt_set_gate(46, (uint32_t)irq14, 0x08, 0x8E);
    idt_set_gate(47, (uint32_t)irq15, 0x08, 0x8E);
}

void irq_register_handler(uint8_t irq_num, isr_handler_t handler) {
    if (irq_num < 16) {
        irq_handlers[irq_num] = handler;
    }
}

void irq_unmask(uint8_t irq_num) {
    uint16_t port;
    uint8_t  bit;

    if (irq_num < 8) {
        port = PIC1_DATA;
        bit  = irq_num;
    } else if (irq_num < 16) {
        port = PIC2_DATA;
        bit  = (uint8_t)(irq_num - 8);
    } else {
        return;
    }

    outb(port, (uint8_t)(inb(port) & ~(1 << bit)));
}

void irq_mask(uint8_t irq_num) {
    uint16_t port;
    uint8_t  bit;

    if (irq_num < 8) {
        port = PIC1_DATA;
        bit  = irq_num;
    } else if (irq_num < 16) {
        port = PIC2_DATA;
        bit  = (uint8_t)(irq_num - 8);
    } else {
        return;
    }

    outb(port, (uint8_t)(inb(port) | (1 << bit)));
}

void irq_handler(registers_t* r) {
    uint8_t irq = (uint8_t)(r->int_no - 32);

    /* Run the registered handler if any. */
    if (irq < 16 && irq_handlers[irq]) {
        irq_handlers[irq](r);
    }

    /* Send End-Of-Interrupt to the PIC(s). */
    if (irq >= 8) {
        outb(PIC2_CMD, PIC_EOI);   /* slave first */
    }
    outb(PIC1_CMD, PIC_EOI);
}
