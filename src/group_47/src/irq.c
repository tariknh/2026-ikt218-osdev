#include "irq.h"
#include "io.h"
#include "idt.h"
#include "terminal.h"
#include "keyboard.h"
#include "pit.h"

// This file contains the implementation of the IRQ (Interrupt Request) handling, which is responsible for managing hardware interrupts 
//from devices like the keyboard, timer, and more. The IRQ system allows us to respond to hardware events in a timely manner, 
//ensuring that our operating system can interact with peripherals effectively. In this implementation, we will set up the IRQs, 
//remap them to avoid conflicts with CPU exceptions, and provide a mechanism for registering custom handlers for specific IRQs.

extern void irq0();  extern void irq1();  extern void irq2();  extern void irq3();
extern void irq4();  extern void irq5();  extern void irq6();  extern void irq7();
extern void irq8();  extern void irq9();  extern void irq10(); extern void irq11();
extern void irq12(); extern void irq13(); extern void irq14(); extern void irq15();

// Array of function pointers to handle IRQs 0 through 15
void *irq_routines[16] = { 0 };

// This lets us "register" a custom handler (like the keyboard)
void irq_install_handler(int irq, void (*handler)(registers_t *r)) {
    if (irq >= 0 && irq < 16) {
        irq_routines[irq] = handler;
    }
}

void irq_uninstall_handler(int irq) {
    if (irq >= 0 && irq < 16) {
        irq_routines[irq] = 0;
    }
}

void irq_remap() {
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    outb(0x21, 0x20); // Remap Master PIC to IDT 32
    outb(0xA1, 0x28); // Remap Slave PIC to IDT 40
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    outb(0x21, 0x00);
    outb(0xA1, 0x00);

    uint8_t master_mask = inb(0x21);
    master_mask = master_mask & ~0x02;  // Clear bit 1 (enable IRQ1)
    outb(0x21, master_mask);
}

void irq_install() {
    irq_remap();

    // Set all IDT gates for IRQ0-IRQ15 (interrupts 32-47)
    idt_set_gate(32, (uint32_t)irq0, 0x08, 0x8E);  // Timer
    idt_set_gate(33, (uint32_t)irq1, 0x08, 0x8E);  // Keyboard
    idt_set_gate(34, (uint32_t)irq2, 0x08, 0x8E);  // Cascade
    idt_set_gate(35, (uint32_t)irq3, 0x08, 0x8E);  // COM2
    idt_set_gate(36, (uint32_t)irq4, 0x08, 0x8E);  // COM1
    idt_set_gate(37, (uint32_t)irq5, 0x08, 0x8E);  // LPT2
    idt_set_gate(38, (uint32_t)irq6, 0x08, 0x8E);  // Floppy
    idt_set_gate(39, (uint32_t)irq7, 0x08, 0x8E);  // LPT1
    idt_set_gate(40, (uint32_t)irq8, 0x08, 0x8E);  // CMOS RTC
    idt_set_gate(41, (uint32_t)irq9, 0x08, 0x8E);  // Free
    idt_set_gate(42, (uint32_t)irq10, 0x08, 0x8E); // Free
    idt_set_gate(43, (uint32_t)irq11, 0x08, 0x8E); // Free
    idt_set_gate(44, (uint32_t)irq12, 0x08, 0x8E); // PS2 mouse
    idt_set_gate(45, (uint32_t)irq13, 0x08, 0x8E); // FPU
    idt_set_gate(46, (uint32_t)irq14, 0x08, 0x8E); // Primary ATA
    idt_set_gate(47, (uint32_t)irq15, 0x08, 0x8E); // Secondary ATA
}

// This is called by our assembly stub
void irq_handler(registers_t *r) {
    int irq = r->int_no - 32;

    if (irq == 0) {
        pit_handler();  
    } 
    else if (irq == 1) {
        keyboard_handler(r);   // Call your dedicated keyboard handler
    }
    
    // Send EOI
    outb(0x20, 0x20);
    if (irq >= 8) outb(0xA0, 0x20);
}