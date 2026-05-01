/*
Name: gdt.c
Project: LocOS
Description: This file contains the implementation of the Global Descriptor Table (GDT) initialization for the LocOS kernel. The GDT is a critical data structure in x86 architecture that defines the characteristics
*/


#include "gdt.h"
#include <libc/stdint.h>

/* A GDT descriptor is 8 bytes */
struct gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_middle;
    uint8_t  access;
    uint8_t  granularity;
    uint8_t  base_high;
} __attribute__((packed));

/* Pointer consumed by lgdt */
struct gdt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

static struct gdt_entry gdt[3];
static struct gdt_ptr   gp;

extern void gdt_load(uint32_t);  /* implemented in gdt.asm */

/* Populate one GDT entry */
static void gdt_set_entry(int i, uint32_t base, uint32_t limit,
                          uint8_t access, uint8_t gran) {
    gdt[i].base_low    = (base & 0xFFFF);
    gdt[i].base_middle = (base >> 16) & 0xFF;
    gdt[i].base_high   = (base >> 24) & 0xFF;
    gdt[i].limit_low   = (limit & 0xFFFF);
    gdt[i].granularity = ((limit >> 16) & 0x0F) | (gran & 0xF0);
    gdt[i].access      = access;
}

void gdt_init(void) {
    gp.limit = (sizeof(struct gdt_entry) * 3) - 1;
    gp.base  = (uint32_t)&gdt;

    gdt_set_entry(0, 0, 0,          0,    0);         /* NULL selector */
    gdt_set_entry(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);      /* 32-bit code */
    gdt_set_entry(2, 0, 0xFFFFFFFF, 0x92, 0xCF);      /* 32-bit data */

    gdt_load((uint32_t)&gp);
}
