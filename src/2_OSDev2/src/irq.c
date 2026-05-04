#include <libc/stdint.h>
#include "irq.h"
#include "idt.h"
#include "io.h"
#include "terminal.h"
#include "keyboard.h"
#include "pit.h"

extern void irq0(void); // Timer interrupt handler
extern void irq1(void); // Keyboard interrupt handler
extern void irq2(void); // Cascade (used internally by the PIC)
extern void irq3(void); // COM2
extern void irq4(void); // COM1
extern void irq5(void); // LPT2
extern void irq6(void); // Floppy disk
extern void irq7(void); // LPT1
extern void irq8(void); // RTC
extern void irq9(void); // Free for peripherals
extern void irq10(void); // Free for peripherals
extern void irq11(void); // Free for peripherals
extern void irq12(void); // PS2 mouse
extern void irq13(void); // FPU
extern void irq14(void); // Primary ATA hard disk
extern void irq15(void); // Secondary ATA hard disk

static void pic_remap(void) {
    // PIC Ports
    const uint16_t PIC1 = 0x20; // Master PIC
    const uint16_t PIC2 = 0xA0; // Slave PIC
    const uint16_t PIC1_COMMAND = PIC1;
    const uint16_t PIC1_DATA = PIC1 + 1;
    const uint16_t PIC2_COMMAND = PIC2;
    const uint16_t PIC2_DATA = PIC2 + 1;

    // Save masks
    uint8_t a1 = inb(PIC1_DATA);
    uint8_t a2 = inb(PIC2_DATA);

    // Start initialization
    outb(PIC1_COMMAND, 0x11); io_wait();
    outb(PIC2_COMMAND, 0x11); io_wait();

    // Set vector offsets
    outb(PIC1_DATA, 0x20); io_wait(); // Master PIC vector offset
    outb(PIC2_DATA, 0x28); io_wait(); // Slave PIC vector offset

    // Tell Master PIC about Slave PIC at IRQ2
    outb(PIC1_DATA, 0x04); io_wait();
    outb(PIC2_DATA, 0x02); io_wait();

    // Set PICs to 8086 mode
    outb(PIC1_DATA, 0x01); io_wait();
    outb(PIC2_DATA, 0x01); io_wait();

    // Restore saved masks
    outb(PIC1_DATA, a1);
    outb(PIC2_DATA, a2);
}

static void pic_send_eoi(uint8_t irq) {
    if (irq >= 8) {
        outb(0xA0, 0x20); // Send EOI to Slave PIC
    }
    outb(0x20, 0x20); // Send EOI to Master
}

void irq_init(void) {
    pic_remap();

    // Set IDT gates 0x20 to 0x2F for IRQs 0-15
    idt_set_gate(0x20, (uint32_t)irq0, 0x08, 0x8E);
    idt_set_gate(0x21, (uint32_t)irq1, 0x08, 0x8E);
    idt_set_gate(0x22, (uint32_t)irq2, 0x08, 0x8E);
    idt_set_gate(0x23, (uint32_t)irq3, 0x08, 0x8E);
    idt_set_gate(0x24, (uint32_t)irq4, 0x08, 0x8E);
    idt_set_gate(0x25, (uint32_t)irq5, 0x08, 0x8E);
    idt_set_gate(0x26, (uint32_t)irq6, 0x08, 0x8E);
    idt_set_gate(0x27, (uint32_t)irq7, 0x08, 0x8E);
    idt_set_gate(0x28, (uint32_t)irq8, 0x08, 0x8E);
    idt_set_gate(0x29, (uint32_t)irq9, 0x08, 0x8E);
    idt_set_gate(0x2A, (uint32_t)irq10, 0x08, 0x8E);
    idt_set_gate(0x2B, (uint32_t)irq11, 0x08, 0x8E);
    idt_set_gate(0x2C, (uint32_t)irq12, 0x08, 0x8E);
    idt_set_gate(0x2D, (uint32_t)irq13, 0x08, 0x8E);
    idt_set_gate(0x2E, (uint32_t)irq14, 0x08, 0x8E);
    idt_set_gate(0x2F, (uint32_t)irq15, 0x08, 0x8E);

    keyboard_init();
}

void irq_handler(isr_frame_t* frame) {
    uint8_t irq = (uint8_t)(frame->int_no - 0x20); // Calculate IRQ number from interrupt number

    if (irq == 0) { // Timer IRQ
        pit_on_irq0(); // Increment tick count
    }
    else if (irq == 1) { // Keyboard IRQ
        keyboard_on_irq1(); // read scancode, buffer, print ASCII
    }

    pic_send_eoi(irq); // Send End of Interrupt signal to PIC
}

