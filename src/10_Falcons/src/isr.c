#include <libc/stdint.h>
#include "isr.h"
#include "idt.h"

extern void isr0(void);
extern void isr1(void);
extern void isr2(void);

extern void terminal_write(const char* data);

void isr_install(void) {
    idt_set_gate(0, (uint32_t)isr0, 0x08, 0x8E);
    idt_set_gate(1, (uint32_t)isr1, 0x08, 0x8E);
    idt_set_gate(2, (uint32_t)isr2, 0x08, 0x8E);
}

void isr_handler(uint32_t interrupt_number) {
    if (interrupt_number == 0) {
        terminal_write("Interrupt 0 triggered: Divide by zero\n");
    } else if (interrupt_number == 1) {
        terminal_write("Interrupt 1 triggered: Debug exception\n");
    } else if (interrupt_number == 2) {
        terminal_write("Interrupt 2 triggered: Non-maskable interrupt\n");
    } else {
        terminal_write("Unknown interrupt triggered\n");
    }
}