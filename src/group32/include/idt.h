#ifndef IDT_H
#define IDT_H
// Includes fixed-size integer types
#include "libc/stdint.h"

// One entry in the Interrupt Descriptor Table
struct idt_entry {
    uint16_t base_low; // Lower 16 bits of handler address
    uint16_t sel; // Kernel code segment selector
    uint8_t always0; // Always set to 0
    uint8_t flags; // Gate type and permissions
    uint16_t base_high; // Upper 16 bits of handler address
} __attribute__((packed));

// Pointer structure used by lidt instruction
struct idt_ptr {
    uint16_t limit; // Size of the IDT
    uint32_t base; // Address of the IDT
} __attribute__((packed));

// Installs the Interrupt Descriptor Table
void idt_install(void);
// Sets one interrupt gate in the IDT
void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags);

#endif