#pragma once

#include "libc/stdint.h"

// we need 3 entries: null, code and data
#define GDT_SIZE 3

// code segment is entry 1, so offset is 1 * 8 = 0x08
#define GDT_KERNEL_CODE 0x08

// data segment is entry 2, so offset is 2 * 8 = 0x10
#define GDT_KERNEL_DATA 0x10

// one entry in the GDT table, describes one memory segment
// has to be exactly 8 bytes so we use packed
// the base and limit are split across multiple fields because of old CPU compatibility
typedef struct {
    uint16_t limit_low;    // lower 16 bits of segment size
    uint16_t base_low;     // lower 16 bits of base address
    uint8_t  base_middle;  // middle 8 bits of base address
    uint8_t  access;       // who can use this segment and how
    uint8_t  granularity;  // upper 4 bits of limit + flags
    uint8_t  base_high;    // upper 8 bits of base address
} __attribute__((packed)) gdt_entry_t;

// tells the CPU where our GDT is in memory
// gets loaded into the CPU with the lgdt instruction
typedef struct {
    uint16_t limit;  // size of GDT minus 1
    uint32_t base;   // address of GDT in memory
} __attribute__((packed)) gdt_descriptor_t;

// sets up the GDT and loads it into the CPU
void gdt_init(void);