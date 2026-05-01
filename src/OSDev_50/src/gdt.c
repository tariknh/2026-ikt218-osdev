#include "gdt.h"
#include <libc/stdint.h>

struct gdt_desc {
    uint16_t limit_low;     // lower 16 bits of limit
    uint16_t base_low;      // lower 16 bits of base
    uint8_t  base_middle;   // next 8 bits of base
    uint8_t  access;        // access flags
    uint8_t  granularity;   // granularity + upper 4 bits of limit
    uint8_t  base_high;     // last 8 bits of base
} __attribute__((packed));

struct gdtr_desc {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

static struct gdt_desc  gdt_table[3];
static struct gdtr_desc gdtr;

static void gdt_set_descriptor(int index, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran_flags)
{
    gdt_table[index].base_low    = (base & 0xFFFF);
    gdt_table[index].base_middle = (base >> 16) & 0xFF;
    gdt_table[index].base_high   = (base >> 24) & 0xFF;

    gdt_table[index].limit_low   = (limit & 0xFFFF);
    gdt_table[index].granularity = (limit >> 16) & 0x0F;

    gdt_table[index].granularity |= (gran_flags & 0xF0);
    gdt_table[index].access      = access;
}

void gdt_init(void)
{
    gdtr.limit = (uint16_t)(sizeof(gdt_table) - 1);
    gdtr.base  = (uint32_t)&gdt_table;

    // 0: NULL
    gdt_set_descriptor(0, 0, 0, 0, 0);

    // 1: CODE (base=0, limit=4GB, access=0x9A, gran=0xCF)
    gdt_set_descriptor(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);

    // 2: DATA (base=0, limit=4GB, access=0x92, gran=0xCF)
    gdt_set_descriptor(2, 0, 0xFFFFFFFF, 0x92, 0xCF);

    // Load GDT og reload segmentregistre i asm
    gdt_flush((uint32_t)&gdtr);
}
