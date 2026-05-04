#include "idt.h"

extern void idt_load(unsigned int); // Assembly function that loads the IDT

static struct idt_entry idt[256]; // IDT with 256 entries
static struct idt_ptr idtp; // Pointer used by lidt

// Sets one IDT gate
void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt[num].base_low = base & 0xFFFF;     // Store lower part of handler address
    idt[num].base_high = (base >> 16) & 0xFFFF;    // Store upper part of handler address

    idt[num].sel = sel;     // Set code segment selector
    idt[num].always0 = 0;    // This field must always be zero
    idt[num].flags = flags;    // Set gate flags
}

// Installs the IDT
void idt_install(void) {
    int i;

    idtp.limit = (sizeof(struct idt_entry) * 256) - 1;     // Set IDT size
    idtp.base = (uint32_t)&idt;     // Set IDT address

    // Clear all IDT entries
    for (i = 0; i < 256; i++) {
        idt_set_gate(i, 0, 0, 0);
    }

    idt_load((unsigned int)&idtp);     // Load the IDT
}