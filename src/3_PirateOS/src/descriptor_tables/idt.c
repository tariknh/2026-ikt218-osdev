#include "descriptor_tables/idt.h"

// Define the IDT with 256 entries.
idt_entry_t idt_entries[256];
idt_ptr_t idt_ptr;

// Declared in interrupts.asm
extern void idt_flush(uint32_t);

void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) // Set an entry in the IDT
{
    idt_entries[num].base_low = base & 0xFFFF;          // Set the lower 16 bits of the base address
    idt_entries[num].base_high = (base >> 16) & 0xFFFF; // Set the higher 16 bits of the base address

    idt_entries[num].sel = sel;     // Set the segment selector
    idt_entries[num].always0 = 0;   // This must always be zero
    idt_entries[num].flags = flags; // Set the flags (type and attributes)
}

void init_idt() // Initialize the IDT
{
    idt_ptr.limit = sizeof(idt_entry_t) * 256 - 1; // Set the limit to the size of the IDT in bytes - 1
    idt_ptr.base = (uint32_t)&idt_entries;         // Set the base to the address of the first element in our idt_entry_t array.

    // Clear the IDT memory to 0
    for (int i = 0; i < 256; i++)
    {
        idt_set_gate(i, 0, 0, 0);
    }

    // Load the IDT pointer
    idt_flush((uint32_t)&idt_ptr);
}
