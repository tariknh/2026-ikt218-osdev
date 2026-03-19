#include "../include/gdt.h"

// The GDT table - holds our 3 segment descriptors
static gdt_entry_t gdt_segments[GDT_SIZE];

// The GDT descriptor - tells the CPU where our table is
static gdt_descriptor_t gdt_descriptor;

// Defined in gdt.asm - loads GDT into the CPU using lgdt instruction
extern void gdt_flush(uint32_t);

/**
 * Encodes a single segment descriptor into the GDT table.
 *
 * The base and limit are split across weird fields due to
 * backwards compatibility with old 286 CPUs. This function
 * handles that splitting for us.
 *
 * Parameters:
 *   slot   - index in the GDT table (0=null, 1=code, 2=data)
 *   base   - start address of the segment (0 = whole memory)
 *   limit  - size of the segment (0xFFFFFF = whole memory)
 *   access - who can use it and how (code? data? kernel only?)
 *   flags  - extra CPU settings (32-bit mode, page granularity)
 */
static void encode_segment(int slot, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags) {
    // Encode the base address - split across 3 fields
    gdt_segments[slot].base_low    = (base & 0xFFFF);
    gdt_segments[slot].base_middle = (base >> 16) & 0xFF;
    gdt_segments[slot].base_high   = (base >> 24) & 0xFF;

    // Encode the limit - split across 2 fields
    gdt_segments[slot].limit_low   = (limit & 0xFFFF);
    gdt_segments[slot].granularity = (limit >> 16) & 0x0F;

    // Pack the flags into the upper 4 bits of granularity
    gdt_segments[slot].granularity |= (flags & 0xF0);

    // Set the access byte
    gdt_segments[slot].access = access;
}

/**
 * Initializes the Global Descriptor Table.
 *
 * Sets up 3 entries as required by the assignment:
 *   0 - Null descriptor  (CPU requires this to always be empty)
 *   1 - Kernel code      (executable code, ring 0, full memory)
 *   2 - Kernel data      (writable data,   ring 0, full memory)
 *
 * Then loads the GDT into the CPU.
 */
void gdt_init(void) {
    // Set up the descriptor pointing to our table
    gdt_descriptor.limit = (sizeof(gdt_entry_t) * GDT_SIZE) - 1;
    gdt_descriptor.base  = (uint32_t)&gdt_segments;

    // Entry 0: Null descriptor - required by CPU, always empty
    encode_segment(0, 0, 0, 0, 0);

    // Entry 1: Kernel code segment
    // Access 0x9A = present | ring 0 | code segment | executable | readable
    // Flags  0xCF = 32-bit protected mode | 4KB page granularity
    encode_segment(1, 0, 0xFFFFFF, 0x9A, 0xCF);

    // Entry 2: Kernel data segment
    // Access 0x92 = present | ring 0 | data segment | writable
    // Flags  0xCF = 32-bit protected mode | 4KB page granularity
    encode_segment(2, 0, 0xFFFFFF, 0x92, 0xCF);

    // Load the GDT into the CPU!
    gdt_flush((uint32_t)&gdt_descriptor);
}