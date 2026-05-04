#include <libc/stdint.h>
#include "idt.h"
#include "irq.h"
#include "ports.h"
#include "terminal.h"

#define PIC1_COMMAND 0x20
#define PIC1_DATA    0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA    0xA1
#define PIC_EOI      0x20

#define ICW1_ICW4    0x01
#define ICW1_INIT    0x10
#define ICW4_8086    0x01

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

static uint8_t irq_seen[16] = {0};
static irq_callback_t irq_callbacks[16] = {0};

static void terminal_write_irq_number(uint32_t value) {
    char buffer[3];

    if (value < 10) {
        buffer[0] = (char)('0' + value);
        buffer[1] = '\0';
        terminal_write(buffer);
        return;
    }

    buffer[0] = '1';
    buffer[1] = (char)('0' + (value - 10));
    buffer[2] = '\0';
    terminal_write(buffer);
}

static void pic_send_eoi(uint8_t irq) {
    if (irq >= 8) {
        port_byte_out(PIC2_COMMAND, PIC_EOI);
    }

    port_byte_out(PIC1_COMMAND, PIC_EOI);
}

static void pic_remap(int offset1, int offset2) {
    uint8_t master_mask = port_byte_in(PIC1_DATA);
    uint8_t slave_mask = port_byte_in(PIC2_DATA);

    /* Unmask timer IRQ0 and keyboard IRQ1 so PIT and input interrupts can fire. */
    master_mask &= ~(1 << 0);
    master_mask &= ~(1 << 1);

    port_byte_out(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();
    port_byte_out(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();

    port_byte_out(PIC1_DATA, (uint8_t)offset1);
    io_wait();
    port_byte_out(PIC2_DATA, (uint8_t)offset2);
    io_wait();

    port_byte_out(PIC1_DATA, 0x04);
    io_wait();
    port_byte_out(PIC2_DATA, 0x02);
    io_wait();

    port_byte_out(PIC1_DATA, ICW4_8086);
    io_wait();
    port_byte_out(PIC2_DATA, ICW4_8086);
    io_wait();

    port_byte_out(PIC1_DATA, master_mask);
    port_byte_out(PIC2_DATA, slave_mask);
}

void register_irq_handler(uint8_t irq_line, irq_callback_t handler) {
    if (irq_line < 16) {
        irq_callbacks[irq_line] = handler;
    }
}

void irq_install(void) {
    pic_remap(0x20, 0x28);

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

void irq_handler(struct registers* regs) {
    uint32_t irq_number;

    if (regs->int_no < 32 || regs->int_no > 47) {
        terminal_write("\nUnexpected IRQ vector.");
        return;
    }

    irq_number = regs->int_no - 32;

    if (irq_callbacks[irq_number] != 0) {
        irq_callbacks[irq_number](regs);
    } else if (!irq_seen[irq_number]) {
        terminal_write("\nIRQ ");
        terminal_write_irq_number(irq_number);
        terminal_write(" received.");
        irq_seen[irq_number] = 1;
    }

    pic_send_eoi((uint8_t)irq_number);
}
