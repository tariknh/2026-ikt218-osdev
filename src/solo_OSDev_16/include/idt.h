#ifndef IDT_H
#define IDT_H

#include <libc/stdint.h>

#define KERNEL_CODE_SEGMENT 0x08 // GDT selector for the kernel code segment 

extern void irq15(void);

// One IDT entry is 8 bytes in 32-bit protected mode.
struct idt_entry{
    uint16_t isr_low;          // Lower 16 bits of the ISR address
    uint16_t selector;         // GDT code segment selector used when entering the ISR
    uint8_t reserved;          // This byte must be zero
    uint8_t flags;             // Type and attributes for interrupt gate
    uint16_t isr_high;         // Upper 16 bits of the ISR address
} __attribute__((packed));     // Prevents compiler padding

// Pointer structure used by lidt
struct idt_ptr{
    uint16_t limit;            // Size of entire IDT - 1
    uint32_t base;             // Address of the first IDT entry
}__attribute__((packed));

void idt_init(void);           // Initializes and loads the IDT



#endif