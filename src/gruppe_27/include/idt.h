#ifndef IDT_H
#define IDT_H

#include <stdint.h>

// The structure of one IDT gate
struct idt_entry {
    uint16_t base_low;    // Lower 16 bits of the handler address
    uint16_t sel;         // Kernel segment selector (usually 0x08)
    uint8_t  always0;     // This must always be zero
    uint8_t  flags;       // Access flags (P, DPL, Type)
    uint16_t base_high;   // Upper 16 bits of the handler address
} __attribute__((packed));

// The pointer the 'lidt' instruction actually uses
struct idt_ptr {
    uint16_t limit;       // Size of the IDT array minus 1
    uint32_t base;        // The address of the first element in the array
} __attribute__((packed));

struct registers {
    uint32_t ds;                                     // Data segment selector
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; // Pushed by pusha
    uint32_t int_no, err_code;                       // Our manual pushes
    uint32_t eip, cs, eflags, useresp, ss;           // Pushed by the processor automatically
};

void idt_init(); // Renamed from idt_install to be more descriptive
void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags);

// The function in kernel.bin assembly
extern void idt_load(struct idt_ptr *ptr);
extern void isr_install(); 

void irq_install_handler(int irq, void (*handler)(struct registers *r));
void irq_install();

void keyboard_handler(struct registers *r);

#endif