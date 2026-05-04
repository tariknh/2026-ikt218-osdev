#pragma once
#include <libc/stdint.h>

typedef struct __attribute__((packed)) {
    uint16_t offset_low;   // Lower 16 bits of the handler function address
    uint16_t selector;     // Code segment selector in GDT
    uint8_t  zero;         // Unused, set to 0
    uint8_t  type_attr;    // Type and attributes of the interrupt gate
    uint16_t offset_high;  // Higher 16 bits of the handler function address
} idt_entry_t;

typedef struct __attribute__((packed)) {
    uint16_t limit;        // Size of the IDT in bytes - 1
    uint32_t base;         // Base address of the IDT
} idt_ptr_t;

void idt_init(void);    // Initializes the IDT and loads it into the CPU
void idt_set_gate(uint8_t vec, uint32_t handler, uint16_t selector, uint8_t type_attr); // Sets an entry in the IDT
