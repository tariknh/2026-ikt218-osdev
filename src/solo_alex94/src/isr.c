#include <libc/stdint.h>
#include "isr.h"
#include "terminal.h"

void isr0_handler(void) {
    terminal_write("\nISR 0 triggered.");
}

void isr1_handler(void) {
    terminal_write("\nISR 1 triggered.");
}

void isr2_handler(void) {
    terminal_write("\nISR 2 triggered.");
}

void isr_handler(struct registers* regs) {
    if (regs->int_no == 0) {
        isr0_handler();
    } else if (regs->int_no == 1) {
        isr1_handler();
    } else if (regs->int_no == 2) {
        isr2_handler();
    } else {
        terminal_write("\nAn interrupt outside Task 2 was triggered.");
    }
}
