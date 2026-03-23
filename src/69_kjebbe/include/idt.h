#ifndef IDT_H
#define IDT_H

#include "libc/stdint.h"

// Struct defines an entry in the interrupt description table IDT
typedef struct {
  uint16_t offset_low; // Offset is the addres of the interrupt service routine
                       // relative to its segment
  uint16_t selector;   // Selector is the GDT entry where the ISR resides. 0x08
  uint8_t zero;        // Reserved bit always zero
  uint8_t type_attr;   // Gate type + 0 DPL + present bit
                       // bit 7 present bit = 1, entry is valid
                       // bits 6-5 DPL = 00, highest privelege level
  // bit 4  = 0, unused value for gate typer other than Task
  // gate.
  // bits 3-0 gate type, = 1110, 32 bit interrupt type.
  uint16_t offset_high;
} __attribute__((packed)) idt_entry_t;

// The IDT pointer, this is loaded with the lidt assembly instruction.
typedef struct {
  uint16_t limit; // Size of the IDT in bytes, subtracted by 1
  uint32_t base;  // Memory address to the start of the IDT
} __attribute__((packed)) idt_ptr_t;

// See the idt.c file for more information.
void idt_set_entry(uint8_t index, uint32_t handler, uint16_t selector,
                   uint8_t type_attr);

// See the idt.c file for more information.
void idt_init(void);

// Defined in the multiboot2.asm assembly file loads the IDT pointer into the
// CPU.
extern void idt_flush(uint32_t idt_ptr_address);

#endif
