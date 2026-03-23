#include "../../include/gdt.h"

// Defines GDT array and pointer to the array
static gdt_entry_t gdt[3];
static gdt_ptr_t gdt_pointer;

// Helper function to populate global descriptor table, since the memory layout
// is akward due to historical reasons.
static void gdt_set_entry(int index, uint32_t base, uint32_t limit,
                          uint8_t access, uint8_t flags) {
  gdt[index].limit_low = limit & 0xFFFF;
  gdt[index].base_low = base & 0xFFFF;
  gdt[index].base_middle = (base >> 16) & 0xFF;
  gdt[index].access = access;
  gdt[index].limit_and_flags = (flags << 4) | ((limit >> 16) & 0x0F);
  gdt[index].base_high = (base >> 24) & 0xFF;
}

void gdt_init(void) {
  // The first GDT entry should always be null
  gdt_set_entry(0, 0, 0, 0, 0);

  // Defines a code segment with access to the entire memory range.
  // access 0x9A,
  // What each bit in the access byte defines can be found in
  // https://wiki.osdev.org/Global_Descriptor_Table
  //
  // The access byte represents the following:
  // Bit 7 present bit = 1, the segment is valid.
  // Bits 6-5 DPL = 0, sets segment to highest privilege.
  // Bit 4 descriptor type = 1, defines a code or datasegment
  // Bit 3 Executable bit = 1 for a code segment.
  // Bit 2 Conforming bit = 0, Code in the segment can only be exectued from the
  // same DPL level.
  // Bit 1 Read = 1, The segment is readable.
  // Bit 0 Accessed bit = 0. The CPU will set this when it uses the segment.
  //
  // flags 0xC
  // Bit 3 = 1 Page granularity, instead of byte granularity.
  // Bit 2 = 1, defines a 32-bit protected mode segment
  // Bit 1 = 0, means the segment is NOT a 64 bit code segment
  // Bit 0 reserved bit,
  // The flags represent:
  gdt_set_entry(1, 0, 0xFFFFF, 0x9A, 0xC);

  // Defines a data segment with access to the entire memory range.
  // access 0x92,
  // The access byte represents the following:
  // Bit 7 present bit = 1, the segment is valid.
  // Bits 6-5 DPL = 0, sets segment to highest privilege.
  // Bit 4 descriptor type = 1, defines a code or datasegment
  // Bit 3 Executable bit = 0 for a data segment.
  // Bit 2 Conforming bit = 0, Code in the segment can only be exectued from the
  // same DPL level.
  // Bit 1 Write = 1, The segment is writable
  // Bit 0  Accessed bit = 0. The CPU will set this when it uses the segment.
  //
  // flags 0xC
  // Same flags as previous entry
  gdt_set_entry(2, 0, 0xFFFFF, 0x92, 0xC);

  // Defines the limit and base of our gdt pointer
  gdt_pointer.limit = sizeof(gdt) - 1;
  gdt_pointer.base = (uint32_t)&gdt;

  // Defined in the multiboot2.asm file
  gdt_flush((uint32_t)&gdt_pointer);
}
