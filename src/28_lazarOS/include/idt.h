#ifndef IDT_H
#define IDT_H

#include <libc/stdint.h>
#define IDT_MAX_DESCRIPTORS 256

struct idt_entry {
    uint16_t    isr_low;      // The lower 16 bits of the ISR's address
    uint16_t    kernel_cs;    // The GDT segment selector that the CPU will load into CS before calling the ISR
    uint8_t     reserved;     // Set to zero
    uint8_t     attributes;   // Type and attributes; see the IDT page
    uint16_t    isr_high;     // The higher 16 bits of the ISR's address
} __attribute__((packed));
typedef struct idt_entry idt_entry_t;

struct idt_descriptor {
    uint16_t    limit;        // The upper 16 bits of all selector limits
    uint32_t    base;         // The address of the first element in our idt_entry_t array
} __attribute__((packed));
typedef struct idt_descriptor idt_descriptor_t;

void idt_init(void);

#endif /* IDT_H */