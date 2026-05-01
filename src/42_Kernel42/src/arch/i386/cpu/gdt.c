#include "arch/i386/cpu/gdt.h"

#include <kernel/log.h>
#include <stdio.h>


// from https://pdos.csail.mit.edu/6.828/2004/readings/i386/s05_01.htm
typedef struct __attribute__((packed)) {
  uint16_t limit_low;
  uint16_t base_low;
  uint8_t base_middle;
  uint8_t access;
  uint8_t granularity;
  uint8_t base_high;
} gdt_entry_t;

// Struct pointing to Global Descriptor Table, basically just the start of it, and the size.
typedef struct __attribute__((packed)) {
  uint16_t limit;
  uint32_t base;
} gdt_ptr_t;

///
/// @param num descriptor number
/// @param base Descriptor base. Location of segment
/// @param limit Descriptor limit. Size of the segment.
/// @param access Access bit, a bit that is set when the segment is accessed. For permissions
/// @param flags Which units should LIMIT be interpreted. For instance for 0xCF sets units to
/// 4 KB.
void gdt_set_entry(int32_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags);

// defined in gdt_routines.asm
extern void gdt_set_gdt(gdt_ptr_t* ptr);
extern void gdt_reload_segments();


// NOTE: https://wiki.osdev.org/GDT_Tutorial

static gdt_entry_t gdt[GDT_ENTRIES];
static gdt_ptr_t gdt_ptr;

void init_gdt() {
  // set gdt limit
  log_info("Initialising GDT with %d entries...\n", GDT_ENTRIES);
  gdt_ptr.limit = sizeof(gdt_entry_t) * GDT_ENTRIES - 1;
  gdt_ptr.base = (uint32_t)gdt;


  // Values taken from https://wiki.osdev.org/GDT_Tutorial
  gdt_set_entry(0, 0, 0, 0, 0);             // Null Descriptor - must be 0
  gdt_set_entry(1, 0, 0xFFFFF, 0x9A, 0xCF); // Kernel mode Code Segment
  // | accessed(0)
  gdt_set_entry(2, 0, 0xFFFFF, 0x92, 0xCF); // Kernel Mode Data Segment
  // writable(1) | accessed(0)
  gdt_set_entry(3, 0, 0xFFFFF, 0xFA, 0xCF); // User Mode Code Segment (base 0, flat model)
  gdt_set_entry(4, 0, 0xFFFFF, 0xF2, 0xCF); // User Mode Data Segment (base 0, flat model)

  // NOTE: TSS is set in tss.c since it needs to be set after paging

  // load gdt
  gdt_set_gdt(&gdt_ptr);
  gdt_reload_segments();
}


void gdt_set_entry(int32_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags) {
  gdt[num].base_low = base & 0xFFFF;
  gdt[num].base_middle = base >> 16 & 0xFF;
  gdt[num].base_high = base >> 24 & 0xFF;

  gdt[num].limit_low = limit & 0xFFFF;

  // Flags (granularity byte) = (Limit High 4 bits) | (Flags High 4 bits)
  gdt[num].granularity = limit >> 16 & 0x0F;
  gdt[num].granularity |= flags & 0xF0;

  gdt[num].access = access;
}

void gdt_reload(void) {
  gdt_set_gdt(&gdt_ptr);
}
