#include <gdt.h>

static gdt_entry_t     gdt[3];
static gdt_descriptor_t gdt_descriptor;

static void gdt_set_entry(int index, uint32_t base, uint32_t limit,
                           uint8_t access, uint8_t granularity)
{
    gdt[index].base_low    = (uint16_t)(base & 0xFFFF);
    gdt[index].base_middle = (uint8_t)((base >> 16) & 0xFF);
    gdt[index].base_high   = (uint8_t)((base >> 24) & 0xFF);

    gdt[index].limit_low   = (uint16_t)(limit & 0xFFFF);
    gdt[index].granularity = (uint8_t)((granularity & 0xF0) | ((limit >> 16) & 0x0F));
    gdt[index].access      = access;
}

void gdt_init(void)
{
    gdt_set_entry(0, 0, 0, 0x00, 0x00);
    gdt_set_entry(1, 0x00000000, 0xFFFFF, 0x9A, 0xC0);
    gdt_set_entry(2, 0x00000000, 0xFFFFF, 0x92, 0xC0);

    gdt_descriptor.limit = (uint16_t)(sizeof(gdt) - 1);
    gdt_descriptor.base  = (uint32_t)&gdt;

    gdt_load(&gdt_descriptor);
    gdt_reload_segments();
}
