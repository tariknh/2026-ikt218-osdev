#pragma once

#include "libc/stdint.h"

/*
 * GDT Entry - 8 bytes per descriptor.
 *
 * Each entry describes a segment: its base address, size (limit),
 * privilege level, and type (code or data).
 *
 * Bit layout (low to high):
 *   Bits  0-15  : limit_low    - lower 16 bits of the 20-bit limit
 *   Bits 16-31  : base_low     - lower 16 bits of the 32-bit base
 *   Bits 32-39  : base_mid     - middle 8 bits of the 32-bit base
 *   Bits 40-47  : access       - present, privilege, type flags
 *   Bits 48-51  : granularity  (lower nibble) upper 4 bits of limit
 *   Bits 52-55  : granularity  (upper nibble) page size / operand size flags
 *   Bits 56-63  : base_high    - upper 8 bits of the 32-bit base
 */
struct gdt_entry {
    uint16_t limit_low;   /* Lower 16 bits of segment limit         */
    uint16_t base_low;    /* Lower 16 bits of segment base address  */
    uint8_t  base_mid;    /* Middle 8 bits of segment base address  */
    uint8_t  access;      /* Access byte: present, DPL, type flags  */
    uint8_t  granularity; /* Upper limit bits + granularity/size flags */
    uint8_t  base_high;   /* Upper 8 bits of segment base address   */
} __attribute__((packed));

/*
 * GDT Pointer (GDTR) - passed to the lgdt instruction.
 *
 * limit : total size of the GDT in bytes, minus 1
 * base  : linear address of the first GDT entry
 */
struct gdt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

/* Segment selector values (index * 8, TI=0, RPL=0) */
#define GDT_SELECTOR_NULL 0x00  /* Null descriptor        */
#define GDT_SELECTOR_CODE 0x08  /* Kernel code segment    */
#define GDT_SELECTOR_DATA 0x10  /* Kernel data segment    */

/* Number of GDT entries: NULL + Code + Data */
#define GDT_ENTRY_COUNT 3

/* Initialise and load the GDT */
void gdt_init(void);
