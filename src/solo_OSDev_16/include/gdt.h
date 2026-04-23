#ifndef GDT_H
#define GDT_H

#include <libc/stdint.h>

// Struct to define one 8-byte GDT entry
struct gdt_entry{
    uint16_t limit_low;         // Lower 16 bits of the segment limit
    uint16_t base_low;          // Lower 16 bits of the base address
    uint8_t base_middle;        // Middle 8 bits of the base address
    uint8_t access;             // Access byte
    uint8_t gran;               // Upper 4 bits of the limit and the flag bits
    uint8_t base_high;          // Highest 8 bits of the base address
} __attribute__((packed));      // Tells the compiler not to add padding bytes to the struct

// Struct used by the lgdt instruction
struct gdt_ptr{
    uint16_t limit;             // Size of the GDT in bytes minus 1
    uint32_t base;              // Address of the first GDT entry
} __attribute__((packed));

void gdt_initialize(void);      // Sets up and loads the GDT

#endif