#ifndef KERNEL_GDT_H
#define KERNEL_GDT_H

#include <libc/stdint.h>

#define GDT_NULL_SELECTOR 0x00
#define GDT_CODE_SELECTOR 0x08
#define GDT_DATA_SELECTOR 0x10

struct GdtEntry {
    uint16_t     limit_low;
    uint16_t     base_low;
    uint8_t      base_middle;
    uint8_t      access;
    uint8_t      granularity;
    uint8_t      base_high;
} __attribute__((packed));

struct GdtDescriptor {
    uint16_t          size;
    uint32_t          offset;
} __attribute__((packed));

void GdtInitialize(void);

#endif