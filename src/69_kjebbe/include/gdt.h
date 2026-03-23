#ifndef GDT_H
#define GDT_H

#include "libc/stdint.h"

// Defines the functions and structs used for initialising a GDT in our
// operating system.

// One entry in the GDT (8 bytes).
//
// The variables defined in the GDT table might be spread, therefore you will
// see a high and low section for the variables limit and base.
typedef struct gdt_entry {
  uint16_t limit_low;  // Limit is the size of the corresponding segment
  uint16_t base_low;   // Base is the starting memory address of the segment.
  uint8_t base_middle; //
  uint8_t access;      // Access byte, hold permissions settings, such as DPL
  uint8_t limit_and_flags; // Upper 4 bits: flags , Lower 4 bits: limit
  uint8_t base_high;
} __attribute__((
    packed)) gdt_entry_t; // packed ensures compiler does not alter the
                          // memory layout of the gdt_entry struct.

// The GDT pointer, this is loaded with the lgdt assembly instruction.
typedef struct {
  uint16_t limit; // Size of GDT in bytes, subtracted by 1
  uint32_t base;  // Memory address to the start of the GDT
} __attribute__((packed)) gdt_ptr_t;

void gdt_init(void);

// Function defined in the multiboot assembly file which loads the
// GDT pointer
extern void gdt_flush(uint32_t gdt_ptr_address);

#endif
