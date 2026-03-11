#include "gdt.h"

struct gdt_entry create_gdt_entry(
    uint32_t base, 
    uint32_t limit, 
    uint8_t access_byte, 
    uint8_t flags) {
    struct gdt_entry r;
    
    // Store lowest 16 bits of base address
    r.base_low = base & 0xFFFF;

    // Store middle 8 bits of base address
    r.base_middle = (base >> 16) & 0xFF;

    // Store highest 8 bits of base address
    r.base_high = (base >> 24) & 0xFF;


    // Store lowest 16 bits of segment limit
    r.limit_low = limit & 0xFFFF;


    // Store the high 4 bits of the segment limit
    // (limit bits 16–19)
    r.granularity = (limit >> 16) & 0x0F;
}