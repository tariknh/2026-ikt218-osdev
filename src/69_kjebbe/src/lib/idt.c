#include "../../include/idt.h"

static idt_entry_t idt[256];
static idt_ptr_t idt_pointer;

// Helper function to populate interrupt descriptor table, since the memory
// layout is akward due to historical reasons.
void idt_set_entry(uint8_t index, uint32_t handler, uint16_t selector,
                   uint8_t type_attr) {
  idt[index].offset_low = handler & 0xFFFF;
  idt[index].selector = selector;
  idt[index].zero = 0;
  idt[index].type_attr = type_attr;
  idt[index].offset_high = (handler >> 16) & 0xFFFF;
}

// Initialises up the interrupt descriptor table and then loads it into the CPU
void idt_init(void) {
  idt_pointer.limit = sizeof(idt) - 1;
  idt_pointer.base = (uint32_t)&idt;

  // Creates an IDT entry for each of the 256 possible entries.
  for (int i = 0; i < 256; i++) {
    // Initalise all entries with invalid 0 handler
    // 0x08 selector which is the code segment.
    // 0x8E 32 bit interrupt gate.
    idt_set_entry(i, 0, 0x08, 0x8E);
  }

  idt_flush((uint32_t)&idt_pointer);
}
