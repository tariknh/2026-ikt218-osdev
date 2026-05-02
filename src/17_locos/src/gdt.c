/*
Name: gdt.c
Project: LocOS
Description: This file contains the implementation of the Global Descriptor Table (GDT) initialization for the LocOS kernel. The GDT is a critical data structure in x86 architecture that defines the characteristics
*/


#include "gdt.h"
#include <libc/stdint.h>

/* A GDT descriptor is 8 bytes */
// One GDT entry with the x86 layout
struct gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_middle;
    uint8_t  access;
    uint8_t  granularity;
    uint8_t  base_high;
} __attribute__((packed));

/* Pointer consumed by lgdt */
// Pointer format used by lgdt
struct gdt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

// Three entries are enough for null, code, and data
static struct gdt_entry gdt[3];
static struct gdt_ptr   gp;

extern void gdt_load(uint32_t);  /* implemented in gdt.asm */

/* Populate one GDT entry */
// Fill one descriptor with base, limit, and flags
static void gdt_set_entry(int i, uint32_t base, uint32_t limit,
                          uint8_t access, uint8_t gran) {
    gdt[i].base_low    = (base & 0xFFFF);
    gdt[i].base_middle = (base >> 16) & 0xFF;
    gdt[i].base_high   = (base >> 24) & 0xFF;
    gdt[i].limit_low   = (limit & 0xFFFF);
    gdt[i].granularity = ((limit >> 16) & 0x0F) | (gran & 0xF0);
    gdt[i].access      = access;
}

// Build the GDT and load it into the CPU
void gdt_init(void) {
    // Point the GDTR at our local table
    gp.limit = (sizeof(struct gdt_entry) * 3) - 1;
    gp.base  = (uint32_t)&gdt;

    // Null, code, and data segments
    gdt_set_entry(0, 0, 0,          0,    0);         /* NULL selector */
    gdt_set_entry(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);      /* 32-bit code */
    gdt_set_entry(2, 0, 0xFFFFFFFF, 0x92, 0xCF);      /* 32-bit data */

    // Load the table with the assembly helper
    gdt_load((uint32_t)&gp);
}
