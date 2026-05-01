#include "idt.h"
#include "isr.h"
#include "irq.h"

#define IDT_ENTRIES 256     
#define IDT_FLAG_INTERRUPT_GATE 0x8E                        

// Assembly function that loads the IDTR register using lidt
extern void idt_load(uint32_t idt_ptr_address);


static struct idt_entry idt[IDT_ENTRIES];           // The IDT table object
static struct idt_ptr idtp;                         // Pointer structure passed to lidt


// Fill one IDT entry 
static void idt_set_gate(uint8_t vector, void *isr, uint8_t flags){
    struct idt_entry *descriptor = &idt[vector];   

    descriptor->isr_low = (uint32_t)isr & 0xFFFF;
    descriptor->selector = KERNEL_CODE_SEGMENT;
    descriptor->reserved = 0;
    descriptor->flags = flags;
    descriptor->isr_high = (uint32_t)isr >> 16;
}


// Initialize the IDT pointer and load it into the CPU
void idt_init(void){
    idtp.limit = sizeof(struct idt_entry) * IDT_ENTRIES - 1;
    idtp.base = (uint32_t)&idt;      
    
    // ISR: 0 - Divide-by-zero, 3 - Breakpoint, 14 - Page fault
    idt_set_gate(0, isr0, IDT_FLAG_INTERRUPT_GATE);
    idt_set_gate(3, isr3, IDT_FLAG_INTERRUPT_GATE);
    idt_set_gate(14, isr14, IDT_FLAG_INTERRUPT_GATE);

    // IRQ 0 - 15
    idt_set_gate(32, irq0, IDT_FLAG_INTERRUPT_GATE);
    idt_set_gate(33, irq1, IDT_FLAG_INTERRUPT_GATE);
    idt_set_gate(34, irq2, IDT_FLAG_INTERRUPT_GATE);
    idt_set_gate(35, irq3, IDT_FLAG_INTERRUPT_GATE);
    idt_set_gate(36, irq4, IDT_FLAG_INTERRUPT_GATE);
    idt_set_gate(37, irq5, IDT_FLAG_INTERRUPT_GATE);
    idt_set_gate(38, irq6, IDT_FLAG_INTERRUPT_GATE);
    idt_set_gate(39, irq7, IDT_FLAG_INTERRUPT_GATE);
    idt_set_gate(40, irq8, IDT_FLAG_INTERRUPT_GATE);
    idt_set_gate(41, irq9, IDT_FLAG_INTERRUPT_GATE);
    idt_set_gate(42, irq10, IDT_FLAG_INTERRUPT_GATE);
    idt_set_gate(43, irq11, IDT_FLAG_INTERRUPT_GATE);
    idt_set_gate(44, irq12, IDT_FLAG_INTERRUPT_GATE);
    idt_set_gate(45, irq13, IDT_FLAG_INTERRUPT_GATE);
    idt_set_gate(46, irq14, IDT_FLAG_INTERRUPT_GATE);
    idt_set_gate(47, irq15, IDT_FLAG_INTERRUPT_GATE);


    idt_load((uint32_t)&idtp);
} 