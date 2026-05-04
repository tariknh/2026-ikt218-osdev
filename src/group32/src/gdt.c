#include "gdt.h"

// Defines one GDT entry
struct gdt_entry {
    unsigned short limit_low;  // Lower 16 bits of the segment limit
    unsigned short base_low; // Lower 16 bits of the base address
    unsigned char base_middle; // Middle 8 bits of the base address
    unsigned char access; // Access flags for the segment
    unsigned char granularity; // Granularity and upper limit bits
    unsigned char base_high; // Upper 8 bits of the base address
} __attribute__((packed));

// Defines the GDT pointer used by lgdt
struct gdt_ptr {
    unsigned short limit; // Size of the GDT minus one
    unsigned int base; // Address of the GDT
} __attribute__((packed));

static struct gdt_entry gdt[3]; // Creates space for three GDT entries
static struct gdt_ptr gp; // Stores the pointer to the GDT


extern void gdt_flush(unsigned int gp_address); // Assembly function that loads the GDT


// Sets one entry in the GDT
static void gdt_set_gate(
    int num,
    unsigned int base,
    unsigned int limit,
    unsigned char access,
    unsigned char gran
) { 
    // Set the base address parts
    gdt[num].base_low = (unsigned short)(base & 0xFFFF);
    gdt[num].base_middle = (unsigned char)((base >> 16) & 0xFF);
    gdt[num].base_high = (unsigned char)((base >> 24) & 0xFF);
    // Set the limit parts
    gdt[num].limit_low = (unsigned short)(limit & 0xFFFF);
    gdt[num].granularity = (unsigned char)((limit >> 16) & 0x0F);
    // Set granularity and access flags
    gdt[num].granularity |= (unsigned char)(gran & 0xF0);
    gdt[num].access = access;
}

// Installs the GDT
void gdt_install(void) {
    gp.limit = (unsigned short)((sizeof(struct gdt_entry) * 3) - 1);    // Set the GDT size

    gp.base = (unsigned int)&gdt;    // Set the GDT address


    gdt_set_gate(0, 0, 0, 0, 0);    // Null descriptor

    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);     // Code segment descriptor

    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);    // Data segment descriptor


    gdt_flush((unsigned int)&gp);    // Load the new GDT

}