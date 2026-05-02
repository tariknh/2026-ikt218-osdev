#include "arch/i386/gdt.h"
#include "stdint.h"

/*
* Global Descriptor Table (GDT)
*
* Reference:
* OSDev Wiki - GDT Tutorial
* https://wiki.osdev.org/GDT_Tutorial
*/

// GDT descriptor stores the base, limit, and access information as one 8-byte segment.
struct gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access;
    uint8_t granularity;
    uint8_t base_high;
} __attribute__((packed));

// GDT pointer stores the table limit and base memory address
// It tells the CPU where the GDT is stored and its size
struct gdt_ptr {
    uint16_t limit; // Last byte in the descriptor table (size)
    uint32_t base; // The memory address of the first gdt_entry
} __attribute__((packed));

static struct gdt_entry gdt[3];
static struct gdt_ptr gdt_descriptor;

// Encodes a GDT descriptor
static void gdt_set_entry(int index, uint32_t base, uint32_t limit, uint8_t access, uint8_t granularity) {
    gdt[index].base_low = (uint16_t) (base & 0xFFFF);
    gdt[index].base_middle = (uint8_t) (base >> 16 & 0xFF);
    gdt[index].base_high = (uint8_t) (base >> 24 & 0xFF);

    gdt[index].limit_low = (uint16_t) (limit & 0xFFFF);
    gdt[index].granularity = (uint8_t) (limit >> 16 & 0x0F);

    gdt[index].granularity |= (uint8_t) (granularity & 0xF0);

    gdt[index].access = access;
}

void gdt_init(void) {
    gdt_descriptor.limit = (uint16_t) (sizeof(gdt) - 1);
    gdt_descriptor.base = (uint32_t) &gdt;

    // NULL descriptor
    gdt_set_entry(0, 0, 0, 0, 0);

    // Kernel Mode Code Segment
    gdt_set_entry(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);

    // Kernel Mode Data Segment
    gdt_set_entry(2, 0, 0xFFFFFFFF, 0x92, 0xCF);

    // Loads the gdt_descriptor into the GDT register
    __asm__ volatile ("lgdt %0" : : "m"(gdt_descriptor));

    // After the gdt_descriptor is loaded the registers must be reloaded
    __asm__ volatile (
        "mov $0x10, %%ax\n"
        "mov %%ax, %%ds\n"
        "mov %%ax, %%es\n"
        "mov %%ax, %%fs\n"
        "mov %%ax, %%gs\n"
        "mov %%ax, %%ss\n"
        "ljmp $0x08, $1f\n"
        "1:\n"
        :
        :
        : "ax", "memory"
    );
}
