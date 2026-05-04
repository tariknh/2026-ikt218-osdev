#include "terminal.h"
#include "libc/stdint.h"

void isr_handler(uint32_t interrupt_number) {
    terminal_print_string("Interrupt ");
    
    if (interrupt_number == 0) {
        terminal_print_string("0");
    } else if (interrupt_number == 1) {
        terminal_print_string("1");
    } else if (interrupt_number == 2) {
        terminal_print_string("2");
    } else {
        terminal_print_string("unknown");
    }

    terminal_print_string(" triggered\n");
}