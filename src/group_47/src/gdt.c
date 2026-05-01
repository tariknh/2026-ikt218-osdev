#include "gdt.h"
// This file contains the implementation of the GDT (Global Descriptor Table) setup and initialization.
extern void gdt_flush(uint32_t gdt_ptr);

gdt_entry_t gdt[3]; // Null, Code, Data
gdt_ptr_t   gdt_ptr; // The GDT pointer structure that we will load into the CPU

// Helper function to set the values correctly into the split bit fields
void gdt_set_gate(int32_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    gdt[num].base_low    = (base & 0xFFFF); // The lowest 16 bits of the base address
    gdt[num].base_middle = (base >> 16) & 0xFF; // The next 8 bits of the base address
    gdt[num].base_high   = (base >> 24) & 0xFF; // The last 8 bits of the base address

    gdt[num].limit_low   = (limit & 0xFFFF); // The lowest 16 bits of the limit
    gdt[num].granularity = (limit >> 16) & 0x0F; // The last 4 bits of the limit
    gdt[num].granularity |= gran & 0xF0; // The granularity flags (size and granularity)
    gdt[num].access      = access; // Access flags (which ring, Code/Data etc)
}

void init_gdt() {
    gdt_ptr.limit = (sizeof(gdt_entry_t) * 3) - 1;
    gdt_ptr.base  = (uint32_t)&gdt;

    gdt_set_gate(0, 0, 0, 0, 0);                // Null segment
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); // Code segment
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF); // Data segment

    gdt_flush((uint32_t)&gdt_ptr);
}