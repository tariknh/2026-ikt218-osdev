#include "idt.h"
#include "terminal.h"

// Define the assembly stubs (from your .asm file)
extern void isr0();
extern void isr1();
extern void isr2();


void isr_install() {
    // Set up the exception gates (0-31)
    idt_set_gate(0, (uint32_t)isr0, 0x08, 0x8E);
    idt_set_gate(1, (uint32_t)isr1, 0x08, 0x8E);
    idt_set_gate(2, (uint32_t)isr2, 0x08, 0x8E);
}

void isr_handler(struct registers r) {
    if (r.int_no == 0) {
        terminal_write("EXCEPTION 0: Division By Zero\n");
    } else if (r.int_no == 1) {
        terminal_write("EXCEPTION 1: Debug\n");
    } else if (r.int_no == 2) {
        terminal_write("EXCEPTION 2: Non Maskable Interrupt\n");
    } else {
        terminal_write("EXCEPTION: Unknown\n");
    }
}