#pragma once
#include "stdint.h"

/** A pointer to the IDT that will be passed to the LIDT instruction. */
struct idt_pointer {
    //! Location in memory
    uint32_t offset;
    //! Size of the IDT-1
    uint16_t size;
} __attribute__((packed));

/** An entry in the idt table.
 * \note This is for 32-bit ONLY
 * \see https://osdev.wiki/wiki/Interrupt_Descriptor_Table#Gate_Descriptor 
 */
struct idt_gate {
    uint16_t low_offset;
    uint16_t selector;
    uint8_t RESERVED_DO_NOT_USE;
    uint8_t attributes;
    uint16_t hi_offset;
} __attribute__((packed));

void load_idt(struct idt_pointer idt_pointer);
struct idt_gate create_idt_gate(uint32_t offset, uint16_t selector, uint8_t attributes);