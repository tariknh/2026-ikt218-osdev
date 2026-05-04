#include "isr.h"
#include "idt.h"
#include "terminal.h"


// These symbols come from isr_stub.asm
extern void isr3(void);
extern void isr4(void);
extern void isr5(void);




static void terminal_write_dec(uint32_t n) {
    char buf[11]; // max 10 digits + '\0'
    int i = 0;

    if (n == 0) {
        terminal_write("0");
        return;
    }

    while (n > 0 && i < 10) {
        buf[i++] = (char)('0' + (n % 10));
        n /= 10;
    }
    buf[i] = '\0';

    // reverse print
    while (i--) {
        char c[2] = { buf[i], 0 };
        terminal_write(c);
    }
}

void isr_handler(uint32_t int_no) {
    terminal_write("ISR triggered: ");
    terminal_write_dec(int_no);
    terminal_write("\n");
}

void isr_install(void) {
    uint16_t cs;
    asm volatile("mov %%cs, %0" : "=r"(cs));

    idt_set_gate(3, (uint32_t)isr3, cs, 0x8E);
    idt_set_gate(4, (uint32_t)isr4, cs, 0x8E);
    idt_set_gate(5, (uint32_t)isr5, cs, 0x8E);
}

