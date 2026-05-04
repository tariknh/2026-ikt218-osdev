#include "terminal.h"
#include "pic.h"
#include "keyboard.h"
#include "pit.h"          
#include "libc/stdint.h"

void irq_handler(uint32_t irq_number) {
    static int timer_printed = 0;

    if (irq_number == 0) {
        pit_irq_handler();

        if (!timer_printed) {
            terminal_print_string("IRQ 0 triggered\n");
            timer_printed = 1;
        }
        return;
    } else if (irq_number == 1) {
        keyboard_handle_input();
    } else if (irq_number == 2) {
        terminal_print_string("IRQ 2 triggered\n");
    } else if (irq_number == 3) {
        terminal_print_string("IRQ 3 triggered\n");
    } else if (irq_number == 4) {
        terminal_print_string("IRQ 4 triggered\n");
    } else if (irq_number == 5) {
        terminal_print_string("IRQ 5 triggered\n");
    } else if (irq_number == 6) {
        terminal_print_string("IRQ 6 triggered\n");
    } else if (irq_number == 7) {
        terminal_print_string("IRQ 7 triggered\n");
    } else if (irq_number == 8) {
        terminal_print_string("IRQ 8 triggered\n");
    } else if (irq_number == 9) {
        terminal_print_string("IRQ 9 triggered\n");
    } else if (irq_number == 10) {
        terminal_print_string("IRQ 10 triggered\n");
    } else if (irq_number == 11) {
        terminal_print_string("IRQ 11 triggered\n");
    } else if (irq_number == 12) {
        terminal_print_string("IRQ 12 triggered\n");
    } else if (irq_number == 13) {
        terminal_print_string("IRQ 13 triggered\n");
    } else if (irq_number == 14) {
        terminal_print_string("IRQ 14 triggered\n");
    } else if (irq_number == 15) {
        terminal_print_string("IRQ 15 triggered\n");
    }

    pic_send_eoi((int)irq_number);
}