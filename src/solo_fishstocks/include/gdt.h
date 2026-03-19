#pragma once

#include "libc/stdint.h"

// Number of GDT entries: null, kernel code, kernel data
#define GDT_SIZE 3

// Kernel code segment selector (entry 1 * 8 bytes = 0x08)
#define GDT_KERNEL_CODE 0x08

// Kernel data segment selector (entry 2 * 8 bytes = 0x10)
#define GDT_KERNEL_DATA 0x10

/**
 * GDT Entry (Segment Descriptor)
 * 
 * Each entry is exactly 8 bytes and describes one memory segment.
 * The base and limit fields are split across multiple parts because
 * of backwards compatibility with old 286 CPUs.
 * 
 * Layout in memory:
 *   [limit_low][base_low][base_middle][access][granularity][base_high]
 *      2 bytes   2 bytes    1 byte     1 byte    1 byte      1 byte
 * 
 * __attribute__((packed)) tells the compiler NOT to add padding between
 * fields - the CPU expects this structure to be exactly 8 bytes!
 */
typedef struct {
    uint16_t limit_low;    // Bits 0-15 of segment size
    uint16_t base_low;     // Bits 0-15 of base address
    uint8_t  base_middle;  // Bits 16-23 of base address
    uint8_t  access;       // Permissions: present, ring level, type
    uint8_t  granularity;  // Bits 16-19 of limit + 4 flag bits
    uint8_t  base_high;    // Bits 24-31 of base address
} __attribute__((packed)) gdt_entry_t;

/**
 * GDT Pointer (GDTR)
 * 
 * This is what gets loaded into the CPU's GDTR register
 * using the lgdt assembly instruction.
 * 
 * It tells the CPU two things:
 *   1. Where the GDT table is in memory (base)
 *   2. How big the GDT table is (limit = size - 1)
 */
typedef struct {
    uint16_t limit;  // Size of the GDT in bytes, minus 1
    uint32_t base;   // Physical memory address of the GDT
} __attribute__((packed)) gdt_descriptor_t;

/**
 * Initializes the Global Descriptor Table.
 * Sets up null, kernel code and kernel data segments,
 * then loads the GDT into the CPU.
 */
void gdt_init(void);