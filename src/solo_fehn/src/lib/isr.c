/*
 * isr.c - C-level handling for CPU exceptions
 *
 * isr_common in isr.asm calls isr_handler() once it has saved registers
 * and switched to the kernel data segment.  Our job here is to:
 *   - report which exception fired
 *   - dispatch to a registered C handler (if any)
 */

#include <isr.h>
#include <libc/stdio.h>

static isr_handler_t isr_handlers[32];

static const char* exception_names[32] = {
    "Division by zero",
    "Debug",
    "Non-maskable interrupt",
    "Breakpoint",
    "Into detected overflow",
    "Out of bounds",
    "Invalid opcode",
    "No coprocessor",
    "Double fault",
    "Coprocessor segment overrun",
    "Bad TSS",
    "Segment not present",
    "Stack fault",
    "General protection fault",
    "Page fault",
    "Reserved",
    "Coprocessor fault",
    "Alignment check",
    "Machine check",
    "Reserved", "Reserved", "Reserved", "Reserved", "Reserved",
    "Reserved", "Reserved", "Reserved", "Reserved", "Reserved",
    "Reserved", "Reserved", "Reserved",
};

void isr_register_handler(uint8_t n, isr_handler_t handler) {
    if (n < 32) {
        isr_handlers[n] = handler;
    }
}

void isr_handler(registers_t* r) {
    if (r->int_no < 32) {
        printf("[ISR] %s (int %d, err %d)\n",
               exception_names[r->int_no],
               (int)r->int_no,
               (int)r->err_code);

        if (isr_handlers[r->int_no]) {
            isr_handlers[r->int_no](r);
        }
    } else {
        printf("[ISR] unknown interrupt %d\n", (int)r->int_no);
    }
}
