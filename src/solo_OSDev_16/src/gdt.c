#include "gdt.h"

#define GDT_ENTRIES 3

// Assembly function that loads the GDTR and reloads segment registers
extern void gdt_flush(uint32_t gdt_ptr_address);

static struct gdt_entry gdt[GDT_ENTRIES];
static struct gdt_ptr gd;

static void gdt_set_gate(int index, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags)
{
    // Encode the base address (32 bits split into 16, 8, 8)
    gdt[index].base_low = base & 0xFFFF;
    gdt[index].base_middle = (base >> 16) & 0xFF;
    gdt[index].base_high = (base >> 24) & 0xFF;

    // Encode the limit (20 bits total: 16 in limit_low, 4 in gran)
    gdt[index].limit_low = limit & 0xFFFF;
    gdt[index].gran = (limit >> 16) & 0x0F;

    // Encode the flags (upper 4 bits of the granularity byte)
    gdt[index].gran |= (flags & 0xF0);

    // Set the access byte
    gdt[index].access = access;
}

void gdt_initialize(void)
{
    gd.limit = (sizeof(struct gdt_entry) * GDT_ENTRIES) - 1;
    gd.base = (uint32_t)&gdt;

    gdt_set_gate(0, 0, 0, 0, 0);                // Null descriptor
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); // Code descriptor
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF); // Data descriptor

    gdt_flush((uint32_t)&gd);
}