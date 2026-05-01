#include "idt.h" // IDT types and init function.
#include "irq.h" // IRQ setup and IRQ handler.

static struct idt_entry idt[256]; //Table with 256 interrupt entries.
static struct idt_ptr   idtp; // Pointer used by lidt.

extern void idt_load(uint32_t); // Assembly code that runs lidt.
extern void isr0(void); // ISR for interrupt 0.
extern void isr1(void); // ISR for interrupt 1.
extern void isr2(void); // ISR for interrupt 2.
extern void isr48(void); // ISR for interrupt 0x30.
extern void irq0(void); // IRQ 0 stub.
extern void irq1(void); // IRQ 1 stub.
extern void irq2(void); // IRQ 2 stub.
extern void irq3(void); // IRQ 3 stub.
extern void irq4(void); // IRQ 4 stub.
extern void irq5(void); // IRQ 5 stub.
extern void irq6(void); // IRQ 6 stub.
extern void irq7(void); // IRQ 7 stub.
extern void irq8(void); // IRQ 8 stub.
extern void irq9(void); // IRQ 9 stub.
extern void irq10(void); // IRQ 10 stub.
extern void irq11(void); // IRQ 11 stub.
extern void irq12(void); // IRQ 12 stub.
extern void irq13(void); // IRQ 13 stub.
extern void irq14(void); // IRQ 14 stub.
extern void irq15(void);           // IRQ 15 stub.
extern void isr_ignore(void); // Default handler for unused vectors.

static void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt[num].base_low  = (uint16_t)(base & 0xFFFF);  // Low half of the handler address.
    idt[num].base_high = (uint16_t)((base >> 16) & 0xFFFF); //High half of the handler address
    idt[num].selector  = sel; // Code segment selector.
    idt[num].zero      = 0;// Reserved byte must stay 0.
    idt[num].flags     = flags; // Gate type and present bit
}

void idt_init(void) {
    idtp.limit = (uint16_t)(sizeof(struct idt_entry) * 256 - 1); // Size of the full table
    idtp.base  = (uint32_t)&idt; // Address of the table.

    for (uint16_t i = 0; i < 256; i++) {
        idt_set_gate((uint8_t)i, (uint32_t)isr_ignore, 0x08, 0x8E); // Fill all of theslots first
    }

    idt_set_gate(0, (uint32_t)isr0, 0x08, 0x8E); // Divide error 
    idt_set_gate(1, (uint32_t)isr1, 0x08, 0x8E); // Debug exception
    idt_set_gate(2, (uint32_t)isr2, 0x08, 0x8E); // NMI vector
    idt_set_gate(0x30, (uint32_t)isr48, 0x08, 0x8E);// Test software interrupt
    idt_set_gate(0x20, (uint32_t)irq0, 0x08, 0x8E);// Timer IRQ
    idt_set_gate(0x21, (uint32_t)irq1, 0x08, 0x8E);// Keyboard IRQ
    idt_set_gate(0x22, (uint32_t)irq2, 0x08, 0x8E);
    idt_set_gate(0x23, (uint32_t)irq3, 0x08, 0x8E);
    idt_set_gate(0x24, (uint32_t)irq4, 0x08, 0x8E);
    idt_set_gate(0x25, (uint32_t)irq5, 0x08, 0x8E);
    idt_set_gate(0x26, (uint32_t)irq6, 0x08, 0x8E);
    idt_set_gate(0x27, (uint32_t)irq7, 0x08, 0x8E);
    idt_set_gate(0x28, (uint32_t)irq8, 0x08, 0x8E);
    idt_set_gate(0x29, (uint32_t)irq9, 0x08, 0x8E);
    idt_set_gate(0x2A, (uint32_t)irq10, 0x08, 0x8E);
    idt_set_gate(0x2B, (uint32_t)irq11, 0x08, 0x8E);
    idt_set_gate(0x2C, (uint32_t)irq12, 0x08, 0x8E);
    idt_set_gate(0x2D, (uint32_t)irq13, 0x08, 0x8E);
    idt_set_gate(0x2E, (uint32_t)irq14, 0x08, 0x8E);
    idt_set_gate(0x2F, (uint32_t)irq15, 0x08, 0x8E);          // IRQ 15.

    irq_init();// Remap and enable PIC lines.
    idt_load((uint32_t)&idtp); // Load IDTR with lidt.
}
