#ifndef GDT_H
#define GDT_H

#include "libc/stdint.h"

// A structure that describes a GDT entry
struct gdt_entry_struct {
    uint16_t limit_low;     // The lowest 16 bits of the limit
    uint16_t base_low;      // The lowest 16 bits of the base address
    uint8_t  base_middle;   // The next 8 bits of the base address
    uint8_t  access;        // Access rights (which ring, Code/Data etc)
    uint8_t  granularity;   // Flag for size and the last 4 bits of the limit
    uint8_t  base_high;     // The last 8 bits of the base address
} __attribute__((packed)); 

typedef struct gdt_entry_struct gdt_entry_t;

// The GDT pointer structure (GDTR) that we load into the CPU
struct gdt_ptr_struct {
    uint16_t limit;               // Biggest index in the GDT
    uint32_t base;                // The address of the first gdt_entry_t
} __attribute__((packed));

typedef struct gdt_ptr_struct gdt_ptr_t;

// Function to initialize the GDT - this is called from kernel.c
void init_gdt();

#endif