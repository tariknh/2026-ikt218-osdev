#ifndef GDT_H
#define GDT_H

#include "libc/stdint.h"

#define GDT_ENTRIES 5

// Structure that defines one segment in the Global Descriptor Table (GDT)
struct gdt_entry {
    uint16_t limit_low;     // Lower 16 bits of the segment limit
    uint16_t base_low;      // Lower 16 bits of the base address
    uint8_t base_middle;    // Middle 8 bits of the base address
    uint8_t access;         // Access flags (permissions and segment type)
    uint8_t granularity;    // Granularity flags + upper 4 bits of limit
    uint8_t base_high;      // Highest 8 bits of the base address
} __attribute__((packed));  // Prevent compiler padding

// Structure that defines the pointer to the GDT, which tells the processor where the GDT is and how big it is 
struct gdt_ptr {
    uint16_t limit;         // Size of the GDT
    uint32_t base;          // Address of the first GDT entry
} __attribute__((packed));

// Set a GDT entry
void gdt_set_gate(int32_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran);

// Load the GDT into the processor
void gdt_load(struct gdt_ptr *gdt_ptr);

// Function in assembly to flush the GDT
extern void gdt_flush(uint32_t gdt_ptr);

// Initialize the GDT
void init_gdt();    

#endif