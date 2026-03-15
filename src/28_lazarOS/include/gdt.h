#ifndef GDT_H
#define GDT_H

#include <libc/stdint.h>

// TODO: Define the GDT entry struct (8 bytes, packed)
// Hint: it needs fields for limit_low, base_low, base_middle,
//       access, granularity, base_high

// TODO: Define the GDT descriptor struct (6 bytes, packed)
// Hint: limit (uint16_t) and base (uint32_t)

// TODO: Declare gdt_init()
// TODO: Extern declare gdt_load() and gdt_reload_segments() from gdt.asm

#endif /* GDT_H */
