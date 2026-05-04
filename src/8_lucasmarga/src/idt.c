#include "idt.h"
#include "gdt.h"
#include "libc/stdint.h"

// Max 256 interrupt entries
static idt_entry_t idt[256];
static idt_ptr_t idt_ptr;

// Function written in assembly 
extern void load_idt(idt_ptr_t* idt_ptr);

// Declaration for ISR handlers
extern void isr0(void);
extern void isr1(void);
extern void isr2(void);

// Declarations for IRQ Handlers 
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


// Setting one IDT descriptor
static void set_idt_entry(uint8_t num, uint32_t base, uint16_t selector, uint8_t flags) {
    idt[num].offset_low  = base & 0xFFFF;
    idt[num].selector    = selector;
    idt[num].zero        = 0;
    idt[num].type_attr   = flags;
    idt[num].offset_high = (base >> 16) & 0xFFFF;
}

void idt_setup(void) {

    idt_ptr.limit = sizeof(idt) - 1;
    idt_ptr.base  = (uint32_t)&idt;

    for (int i = 0; i < 256; i++) {
        set_idt_entry(i, 0, 0, 0);
    }

    // Three interrupt gates
    set_idt_entry(0, (uint32_t)isr0, 0x08, 0x8E);
    set_idt_entry(1, (uint32_t)isr1, 0x08, 0x8E);
    set_idt_entry(2, (uint32_t)isr2, 0x08, 0x8E);


    // Setup IRQ entries 
    set_idt_entry(32, (uint32_t)irq0,  0x08, 0x8E);
    set_idt_entry(33, (uint32_t)irq1,  0x08, 0x8E);
    set_idt_entry(34, (uint32_t)irq2,  0x08, 0x8E);
    set_idt_entry(35, (uint32_t)irq3,  0x08, 0x8E);
    set_idt_entry(36, (uint32_t)irq4,  0x08, 0x8E);
    set_idt_entry(37, (uint32_t)irq5,  0x08, 0x8E);
    set_idt_entry(38, (uint32_t)irq6,  0x08, 0x8E);
    set_idt_entry(39, (uint32_t)irq7,  0x08, 0x8E);
    set_idt_entry(40, (uint32_t)irq8,  0x08, 0x8E);
    set_idt_entry(41, (uint32_t)irq9,  0x08, 0x8E);
    set_idt_entry(42, (uint32_t)irq10, 0x08, 0x8E);
    set_idt_entry(43, (uint32_t)irq11, 0x08, 0x8E);
    set_idt_entry(44, (uint32_t)irq12, 0x08, 0x8E);
    set_idt_entry(45, (uint32_t)irq13, 0x08, 0x8E);
    set_idt_entry(46, (uint32_t)irq14, 0x08, 0x8E);
    set_idt_entry(47, (uint32_t)irq15, 0x08, 0x8E);

    // Loading the IDT
    load_idt(&idt_ptr);
}