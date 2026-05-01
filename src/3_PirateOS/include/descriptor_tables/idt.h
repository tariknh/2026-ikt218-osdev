#ifndef IDT_H
#define IDT_H

#include "libc/stdint.h"

#define IDT_ENTRIES 256

// Interrupt gate descriptor
struct idt_entry
{
    uint16_t base_low;  // The lower 16 bits of the ISR's address
    uint16_t sel;       // The GDT segment selector that the CPU will load into CS before calling the ISR
    uint8_t always0;    // This must always be zero
    uint8_t flags;      // Flags (type and attributes)
    uint16_t base_high; // The higher 16 bits of the ISR's address
} __attribute__((packed));

typedef struct idt_entry idt_entry_t;

// Structure describing a pointer to an array of IDT entries.
// Passed to `lidt` instruction.
struct idt_ptr
{
    uint16_t limit;        // The size of the IDT in bytes - 1
    uint32_t base;         // The address of the first element in our idt_entry_t array.
} __attribute__((packed)); // The `packed` attribute tells the compiler not to add any padding between the fields of the structure.

typedef struct idt_ptr idt_ptr_t;

// Initialize IDT
void init_idt();

// Set an entry in the IDT
void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags);

#endif // IDT_H
