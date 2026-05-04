#include "gdt.h"

#define GDT_ENTRY_COUNT 3

static struct gdt_entry gdt_entries[GDT_ENTRY_COUNT];
static struct gdt_ptr   gdt_pointer;

//points to GDT entry and fills in with the given values
static void gdt_set_entry(int index, uint32_t base, uint32_t limit,
                           uint8_t access, uint8_t granularity) {
    uint8_t* target = (uint8_t*)&gdt_entries[index];

    //writes size (limit) into entry (split across bytes per hardware spec)
    target[0] = limit & 0xFF;
    target[1] = (limit >> 8) & 0xFF;
    target[6] = (limit >> 16) & 0x0F;

    //writes base address into entry (split across bytes per hardware spec)
    target[2] = base & 0xFF;
    target[3] = (base >> 8) & 0xFF;
    target[4] = (base >> 16) & 0xFF;
    target[7] = (base >> 24) & 0xFF;

    //writes access byte and granularity flags
    target[5] = access;
    target[6] |= (granularity & 0xF0);
}

void init_gdt(void) {
    gdt_pointer.limit = (sizeof(struct gdt_entry) * GDT_ENTRY_COUNT) - 1; //size of table -1 for lgdt
    gdt_pointer.base  = (uint32_t)&gdt_entries; //stores table memory address

    //null descriptor (required by CPU, must always be first, all zeros no data)
    gdt_set_entry(0, 0, 0, 0, 0);

    //kernel code segment (executable and readable, ring 0, covers full 4GB)
    gdt_set_entry(1, 0, 0xFFFFF, 0x9A, 0xCF);

    //kernel data segment (writable, ring 0, covers full 4GB)
    gdt_set_entry(2, 0, 0xFFFFF, 0x92, 0xCF);

    //loads GDT and updates segment registers (gdt.asm)
    gdt_flush(&gdt_pointer);
}