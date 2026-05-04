#include <libc/stdint.h>
#include "idt.h"

static idt_entry_t idt[256];  // IDT with 256 entries
static idt_ptr_t idtr;      // Pointer to the IDT

extern void idt_load(uint32_t idtr_addr); // Assembly function to load the IDT

void idt_set_gate(uint8_t vec, uint32_t handler, uint16_t selector, uint8_t type_attr) {
    idt[vec].offset_low = handler & 0xFFFF;         // Set lower 16 bits of handler address
    idt[vec].selector = selector;                    // Set code segment selector
    idt[vec].zero = 0;                              // Unused, set to 0
    idt[vec].type_attr = type_attr;                // Set type and attributes
    idt[vec].offset_high = (handler >> 16) & 0xFFFF; // Set higher 16 bits of handler address
}

void idt_init(void) {
    idtr.limit = sizeof(idt) - 1; // Set the limit to the size of the IDT
    idtr.base = (uint32_t)&idt[0];    // Set the base address to the IDT array

    // Clear the IDT by setting all entries to zero
    for (int i = 0; i < 256; i++) {
        idt_set_gate(i, 0, 0, 0);
    }

    idt_load((uint32_t)&idtr); // Load the IDT into the CPU
}