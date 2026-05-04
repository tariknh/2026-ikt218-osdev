#pragma once
#include "libc/stdint.h"

// Single IDT entry 
typedef struct __attribute__((packed)) {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t  zero;
    uint8_t  type_attr;
    uint16_t offset_high;
} idt_entry_t;

// Stores the IDT pointer 
typedef struct __attribute__((packed)) {
    uint16_t limit;
    uint32_t base;
} idt_ptr_t;

// Main setup function 
void idt_setup(void);