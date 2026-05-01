#include "idt.h"
#include "libc/stdint.h"
// This file contains the implementation of the Interrupt Descriptor Table (IDT) setup, which is crucial 
//for handling interrupts and exceptions in our operating system. The IDT allows us to define how the CPU should 
//respond to various interrupts, such as hardware interrupts from devices or software exceptions like divide-by-zero errors.
// The actual IDT array and the special pointer for the CPU
struct idt_entry idt[256];
struct idt_ptr idtp;

// This is an external assembly function we will write in the next step
extern void idt_load();
extern void isr0();  // Exception 0: Divide by zero
extern void isr1();  // Exception 1: Debug
extern void isr2();  // Exception 2: Non-maskable interrupt (NMI)
extern void irq0();  // IRQ 0: Timer
extern void irq1();  // IRQ 1: Keyboard
extern void irq2();  // IRQ 2: Cascade for IRQs 8-15


void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    // The address of the handler is split into high and low 16-bit parts
    idt[num].base_low = (base & 0xFFFF);
    idt[num].base_high = (base >> 16) & 0xFFFF;
    idt[num].sel     = sel;
    idt[num].always0 = 0;
    idt[num].flags   = flags;
}

void idt_install() {
    // 1. Set up the IDT pointer
    idtp.limit = (sizeof(struct idt_entry) * 256) - 1;
    idtp.base  = (uint32_t)&idt;

    // 2. Clear out the IDT, initializing it to zeros
    for(int i = 0; i < 256; i++) {
        idt_set_gate(i, 0, 0, 0);
    }

    // 3. Point entry 0 (Divide by Zero) to a handler
    idt_set_gate(0, (uint32_t)isr0, 0x08, 0x8E);
    idt_set_gate(1, (uint32_t)isr1, 0x08, 0x8E);
    idt_set_gate(2, (uint32_t)isr2, 0x08, 0x8E);  // Cascade for IRQs

    idt_set_gate(32, (uint32_t)irq0, 0x08, 0x8E);  // Timer
    idt_set_gate(33, (uint32_t)irq1, 0x08, 0x8E);  // Keyboard 
    idt_set_gate(34, (uint32_t)irq2, 0x08, 0x8E);  // IRQ2: Cascade

    // 4. Load the IDT into the CPU register
    idt_load();
}

// Debugging function to print the contents of an IDT entry (for testing purposes)
void idt_print_entry(uint8_t num) {
    print("Entry ");
    print_int(num);
    print(": base=");
    print_int(idt[num].base_low | (idt[num].base_high << 16));
    print(" sel=");
    print_int(idt[num].sel);
    print(" flags=");
    print_int(idt[num].flags);
    print("\n");
}