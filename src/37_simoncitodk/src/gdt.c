#include "gdt.h"

static struct gdt_entry gdt[3];
struct gdt_ptr gp;

extern void gdt_flush(void);

static void gdt_set_gate(int number, uint32_t base, uint32_t limit, uint8_t access, uint8_t granularity)
{
    gdt[number].base_low = base & 0xFFFF;
    gdt[number].base_middle = (base >> 16) & 0xFF;
    gdt[number].base_high = (base >> 24) & 0xFF;

    gdt[number].limit_low = limit & 0xFFFF;
    gdt[number].granularity = (limit >> 16) & 0x0F;

    gdt[number].granularity |= granularity & 0xF0;
    gdt[number].access = access;
}

void gdt_init(void)
{
    gp.limit = sizeof(gdt) - 1;
    gp.base = (uint32_t)&gdt;

    gdt_set_gate(0, 0, 0, 0, 0);
    gdt_set_gate(1, 0, 0xFFFFF, 0x9A, 0xCF);
    gdt_set_gate(2, 0, 0xFFFFF, 0x92, 0xCF);

    gdt_flush();
}