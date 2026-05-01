/*
Name: idt.h
Project: LocOS
Description: This file contains the definitions and function declarations for the Interrupt Descriptor Table (IDT)
*/


#ifndef IDT_H 
#define IDT_H  

#include <libc/stdint.h> // Fixed-width integer types that are used by desciptor fields

struct idt_entry { // CPU gate descriptor format for one IDT vector.
    uint16_t base_low;  // ISR handler address bits 0..15.
    uint16_t selector; // Code segment selector in GDT (typically kernel code segment).
    uint8_t  zero; // Reserved byte. It must be zero for 32-bit interrupt gates.
    uint8_t  flags; // Type, privilege level, and present bit (0x8E).
    uint16_t base_high; // ISR handler address bits 16..31.
} __attribute__((packed)); // Keep exact hardware layout .

struct idt_ptr { // Structure consumed by lidt instruction.
    uint16_t limit;  // Size of the IDT in bytes minus 1.
    uint32_t base;  // Linear address of first entry in the IDT array.
} __attribute__((packed)); // Must match x86 IDTR operand layout exactly.

void idt_init(void);  // Builds the IDT entries, remaps all IRQs, and loads in IDTR.

#endif                           
