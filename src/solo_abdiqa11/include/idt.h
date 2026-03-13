#ifndef IDT_H
#define IDT_H

#include <stdint.h>

#define IDT_ENTRIES 256


// An IDT entry (32-bit interrupt gate)
typedef struct __attribute__((packed)) {
    uint16_t base_low;   // lower 16 bits of handler function address
    uint16_t sel;        // kernel segment selector
    uint8_t  always0;    // always 0
    uint8_t  flags;      // type + attributes
    uint16_t base_high;  // upper 16 bits of handler function address
} idt_entry_t;

// IDTR structure
typedef struct __attribute__((packed)) {
    uint16_t limit;
    uint32_t base;
} idt_ptr_t;


extern idt_ptr_t idtp;

void idt_init(void);
void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags);
void idt_load(idt_ptr_t *ptr);


#endif
