#pragma once

#include "libc/stdint.h"

struct gdt_entry {
    uint16_t limit_low;   /* Segment limit, bits 0–15  */
    uint16_t base_low;    /* Base address,  bits 0–15  */
    uint8_t  base_middle; /* Base address,  bits 16–23 */
    uint8_t  access;      /* Access flags              */
    uint8_t  granularity; /* Granularity flags + limit bits 16–19 */
    uint8_t  base_high;   /* Base address,  bits 24–31 */
} __attribute__((packed));


struct gdt_descriptor {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

#define GDT_KERNEL_CODE 0x08   /* Descriptor index 1 */
#define GDT_KERNEL_DATA 0x10   /* Descriptor index 2 */


void gdt_init(void);
