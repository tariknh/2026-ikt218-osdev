#include "stdint.h"

struct gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access;
    uint8_t granularity;
    uint8_t base_high;
} __attribute__((packed));

struct gdt_pointer {
    uint16_t limit;
    uint32_t address;
} __attribute__((packed));

struct gdt_entry create_gdt_entry(uint32_t base, uint32_t limit, uint8_t access_byte, uint8_t flags);