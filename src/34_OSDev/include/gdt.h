#ifndef GDT_H
#define GDT_H

#include <libc/stdint.h>

struct gdt_entry {
    uint16_t limit_low;    //bit 0-15 of the segment limit
    uint16_t base_low;    //bit 0-15 of the base address
    uint8_t  base_middle;  //bit 16-23 of the base address
    uint8_t  access;       //access flags (present, privilege, type)
    uint8_t  granularity;  //granularity flag + bit 16-19 of limit
    uint8_t  base_high;    //bit 24-31 of the base address
} __attribute__((packed)); //no padding cause CPU expects bytes in exact positions

//GDT pointer is passed to lgdt which tells CPU location and size of GDT
struct gdt_ptr {
    uint16_t limit; //size of GDT (bytes-1)
    uint32_t base;  //memory address of GDT
} __attribute__((packed));

extern void gdt_flush(struct gdt_ptr* ptr);

void init_gdt(void);

#endif