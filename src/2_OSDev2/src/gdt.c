#include <libc/stdint.h>

#include "gdt.h"

struct __attribute__((packed)) gdt_entry {  // Define the structure of a GDT entry
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access;
    uint8_t granularity;
    uint8_t base_high;
};

struct __attribute__((packed)) gdt_ptr {    // Define the structure of the GDT pointer
    uint16_t limit;
    uint32_t base;
};

static struct gdt_entry gdt[3]; // GDT with 3 entries: null, code, and data segments
static struct gdt_ptr gdtr; // GDT pointer to be loaded into the GDTR register

extern void gdt_flush(uint32_t gdtr_addr);  // External function implemented in assembly to load the GDT using the LGDT instruction

static void set_entry(int i, uint32_t base, uint32_t limit, uint8_t access, uint8_t granularity) { // Helper function to set up a GDT entry
    gdt[i].base_low = (uint16_t)(base & 0xFFFF);    // Base address is split into three parts: low, middle, and high
    gdt[i].base_middle = (uint8_t)((base >> 16) & 0xFF);
    gdt[i].base_high = (uint8_t)((base >> 24) & 0xFF);

    gdt[i].limit_low = (uint16_t)(limit & 0xFFFF);  // Limit is split into two parts: low and high (the high part is stored in the granularity byte)
    gdt[i].granularity = (uint8_t)((limit >> 16) & 0x0F);
    gdt[i].granularity |= (granularity & 0xF0);
    gdt[i].access = access; // Access byte defines the segment type and permissions
}

void gdt_init(void) {   // Initialize the GDT pointer and set up the GDT entries
    gdtr.limit = (uint16_t)(sizeof(gdt) - 1);
    gdtr.base = (uint32_t)&gdt[0];

    set_entry(0, 0, 0, 0, 0);                // Null segment
    set_entry(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); // Code segment
    set_entry(2, 0, 0xFFFFFFFF, 0x92, 0xCF); // Data segment

    gdt_flush((uint32_t)&gdtr);
}