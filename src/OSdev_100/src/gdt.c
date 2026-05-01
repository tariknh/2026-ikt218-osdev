#include "../include/gdt.h"

// Telling the compoiler to reserve meemory to hold 3 gdt struvctures.
// and also tell the cpu whre the gdt is, and how big it is.
static gdt_entry_t gdt_segments[GDT_SIZE];
static gdt_descriptor_t gdt_descriptor;


// this is defined in gdt.asm, it loads the GDT into the CPU
extern void gdt_flush(uint32_t);

// fills in one slot in the GDT table
// the base and limit have to be split across multiple fields
static void encode_segment(int slot, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags) {
    // split the base address into 3 parts
    gdt_segments[slot].base_low    = (base & 0xFFFF);
    gdt_segments[slot].base_middle = (base >> 16) & 0xFF;
    gdt_segments[slot].base_high   = (base >> 24) & 0xFF;

    // split the limit into 2 parts
    gdt_segments[slot].limit_low   = (limit & 0xFFFF);
    gdt_segments[slot].granularity = (limit >> 16) & 0x0F;

    // put the flags in the upper bits of granularity
    gdt_segments[slot].granularity |= (flags & 0xF0);

    // set the access byte
    gdt_segments[slot].access = access;
}


// sets up the GDT with 3 entries and loads it into the CPU
void gdt_init(void) {
    // tell the CPU how big our table is and where it is
    gdt_descriptor.limit = (sizeof(gdt_entry_t) * GDT_SIZE) - 1;
    gdt_descriptor.base  = (uint32_t)&gdt_segments;

    // slot 0: null descriptor, CPU requires this to always be empty
    encode_segment(0, 0, 0, 0, 0);

    // slot 1: kernel code segment
    // 0x9A = present, ring 0, code, executable, readable
    // 0xCF = 32-bit mode, 4KB page size
    encode_segment(1, 0, 0xFFFFF, 0x9A, 0xCF);

    // slot 2: kernel data segment
    // 0x92 = present, ring 0, data, writable
    // 0xCF = 32-bit mode, 4KB page size
    encode_segment(2, 0, 0xFFFFF, 0x92, 0xCF);

    // load the GDT into the CPU
    gdt_flush((uint32_t)&gdt_descriptor);
}