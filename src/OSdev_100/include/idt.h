#pragma once

#include "libc/stdint.h"

#define IDT_MAX_DESCRIPTORS    256
#define GDT_OFFSET_KERNEL_CODE 0x08  // GDT slot 1 (1 * 8 bytes)

// The layout of one IDT entry
// where each entry stores lower 16 bits of handler address, segment selector, and some flags
typedef struct {
    uint16_t isr_low;    // Handler address bits 0-15
    uint16_t kernel_cs;  // Code segment selector
    uint8_t  reserved;   // Must be zero
    uint8_t  attributes; // Gate type + privilege flags
    uint16_t isr_high;   // Handler address bits 16-31
} __attribute__((packed)) idt_entry_t;

// Load the IDT into the IDTR register via lidt - hand it to cpu so it knows it exists,
typedef struct {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) idtr_t;


// helper to fill one entry in the IDT table
void idt_init(void);
void idt_set_descriptor(uint8_t vector, void* isr, uint8_t flags);