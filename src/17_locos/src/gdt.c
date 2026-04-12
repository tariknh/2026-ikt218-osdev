/*
Name: gdt.c
Project: LocOS
Description: This file contains the implementation of the Global Descriptor Table (GDT) initialization for the LocOS kernel. The GDT is a critical data structure in x86 architecture that defines the characteristics
*/


#include "gdt.h" //Includes the header file for GDT  with structure definitions and function declarations

#define GDT_ENTRIES 3 // Sets number of GDT entries to 3 (Null, Code, Data) which is what the task asks the group

static gdt_entry_t gdt[GDT_ENTRIES]; // Declares array of 3 GDT entries that are each 8 bytes
static gdt_ptr_t gdt_ptr; // Declares the GDT pointer structure

 // helper function to populate a single GDT entry with given paramterers.
static void gdt_set_entry(int index, uint32_t base, uint32_t limit, uint8_t access, uint8_t granularity) { 
    gdt[index].base_low    = (base & 0xFFFF); // Extracts and stores bits in 0-15 of base address
    gdt[index].base_middle = (base >> 16) & 0xFF; // Extracts and stores bits 16-23 of base address
    gdt[index].base_high   = (base >> 24) & 0xFF; // Extracts and stores bits 24-31 of base address

    gdt[index].limit_low   = (limit & 0xFFFF); // Extracts and stores bits 0-15 of segment limit
    gdt[index].granularity = ((limit >> 16) & 0x0F); // Extracts and stores bits 16-19 of segment limit in the low 4 bits of granularity

    gdt[index].granularity |= (granularity & 0xF0); // ORs upper nibble of granularity parameter into the existing value
    gdt[index].access = access; //Stores access byte
}

void gdt_init(void) { //Main GDT initialization function that is called from the kernel.c
    gdt_ptr.limit = sizeof(gdt_entry_t) * GDT_ENTRIES - 1; //calculates GDT size in bytes minus 1 which is required in x86
    gdt_ptr.base  = (uint32_t)&gdt; //stores address of GDT array in a pointer structure

    // Null descriptor
    gdt_set_entry(0, 0, 0, 0, 0);

    // Code segment: base=0, limit=4GB, ring 0, executable, readable
    gdt_set_entry(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);

    // Data segment: base=0, limit=4GB, ring 0, writable
    gdt_set_entry(2, 0, 0xFFFFFFFF, 0x92, 0xCF);

    gdt_load((uint32_t)&gdt_ptr); // Calls assembly function in gdt.asm to load the GDT. This will be talked about further
    // In that file
}