#include "gdt.h"

//Static GDT with three entries: null, code, and data.
static struct gdt_entry gdt[3];

//Pointer structure passed to the lgdt instruction.
static struct gdt_pointer gdtp;

struct gdt_entry create_gdt_entry(
    uint32_t base,
    uint32_t limit,
    uint8_t access_byte,
    uint8_t flags
) {
    struct gdt_entry r;

    //Split the 32-bit limit and base into the fields used by the x86 descriptor format.
    r.limit_low   = (uint16_t)(limit & 0xFFFFU);
    r.base_low    = (uint16_t)(base & 0xFFFFU);
    r.base_middle = (uint8_t)((base >> 16) & 0xFFU);
    r.access      = access_byte;
    r.granularity = (uint8_t)(((limit >> 16) & 0x0FU) | (flags & 0xF0U));
    r.base_high   = (uint8_t)((base >> 24) & 0xFFU);

    return r;
}

struct gdt_pointer create_gdt_pointer(struct gdt_entry *start, uint32_t entries) {
    struct gdt_pointer r;

    //lgdt expects the size of the table in bytes minus one.
    r.limit = (uint16_t)(entries * sizeof(struct gdt_entry) - 1U);

    //Address of the first descriptor in the table.
    r.address = (uint32_t)start;

    return r;
}

void gdt_init(void) {
    //Entry 0 must be the null descriptor.
    gdt[0] = create_gdt_entry(0x00000000U, 0x00000000U, 0x00U, 0x00U);

    //Entry 1: kernel code segment. Selector = 0x08.
    gdt[1] = create_gdt_entry(0x00000000U, 0x000FFFFFU, 0x9AU, 0xC0U); // kernel code

    //Entry 2: kernel data segment. Selector = 0x10.
    gdt[2] = create_gdt_entry(0x00000000U, 0x000FFFFFU, 0x92U, 0xC0U); // kernel data

    //Build the pointer structure used by lgdt.
    gdtp = create_gdt_pointer(gdt, 3U);

    
    //Load the new GDT, then reload segment registers.
    // CS cannot be written directly, so a far return is used to reload it.
    __asm__ __volatile__(
        "lgdt %0\n\t"
        "movw $0x10, %%ax\n\t"
        "movw %%ax, %%ds\n\t"
        "movw %%ax, %%es\n\t"
        "movw %%ax, %%fs\n\t"
        "movw %%ax, %%gs\n\t"
        "movw %%ax, %%ss\n\t"
        "pushl $0x08\n\t"
        "pushl $1f\n\t"
        "lret\n\t"
        "1:\n\t"
        :
        : "m"(gdtp)
        : "ax", "memory"
    );
}
