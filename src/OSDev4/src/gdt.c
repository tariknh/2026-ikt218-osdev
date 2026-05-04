#include "gdt.h"

#define GDT_ENTRIES 3

static gdt_entry_t gdt[GDT_ENTRIES];
static gdt_ptr_t   gdt_ptr;

extern void gdt_flush(gdt_ptr_t* ptr);

static void encodeGdtEntry(gdt_entry_t* target, struct GDT source)
{
    if (source.limit > 0xFFFFF) return;

    target->limit_low   =  source.limit        & 0xFFFF;
    target->limit_flags = (source.limit >> 16) & 0x0F;
    target->limit_flags |= (source.flags << 4) & 0xF0;

    target->base_low  =  source.base        & 0xFFFF;
    target->base_mid  = (source.base >> 16) & 0xFF;
    target->base_high = (source.base >> 24) & 0xFF;

    target->access = source.access_byte;
}

void gdt_init(void)
{
    encodeGdtEntry(&gdt[0], (struct GDT){ 0, 0,      0x00, 0x0 });
    encodeGdtEntry(&gdt[1], (struct GDT){ 0, 0xFFFFF, 0x9A, 0xC });
    encodeGdtEntry(&gdt[2], (struct GDT){ 0, 0xFFFFF, 0x92, 0xC });

    gdt_ptr.limit = sizeof(gdt) - 1;
    gdt_ptr.base  = (uint32_t)&gdt;

    gdt_flush(&gdt_ptr);
}
