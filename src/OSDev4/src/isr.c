#include "isr.h"
#include <stdint.h>

extern void terminal_write(const char* str);
extern void terminal_putchar(char c);

static void print_uint(uint32_t n)
{
    if (n == 0) {
        terminal_putchar('0');
        return;
    }
    char buf[12];
    int  pos = 0;
    while (n > 0) {
        buf[pos++] = (char)('0' + (n % 10));
        n /= 10;
    }
    while (pos > 0) {
        terminal_putchar(buf[--pos]);
    }
}

static const char* exception_names[32] = {
    "Division by Zero",
    "Debug",
    "Non-Maskable Interrupt",
    "Breakpoint",
    "Overflow",
    "Bound Range Exceeded",
    "Invalid Opcode",
    "Device Not Available",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Invalid TSS",
    "Segment Not Present",
    "Stack Segment Fault",
    "General Protection Fault",
    "Page Fault",
    "Reserved",
    "x87 FPU Exception",
    "Alignment Check",
    "Machine Check",
    "SIMD FP Exception",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved"
};

void isr_handler(registers_t* regs)
{
    if (regs->int_no < 32) {
        terminal_write("\n[EXCEPTION] ");
        terminal_write(exception_names[regs->int_no]);
        terminal_write(" (int ");
        print_uint(regs->int_no);
        terminal_putchar(')');
        if (regs->err_code != 0 ||
            regs->int_no == 8  || regs->int_no == 10 ||
            regs->int_no == 11 || regs->int_no == 12 ||
            regs->int_no == 13 || regs->int_no == 14 ||
            regs->int_no == 17)
        {
            terminal_write(", err=");
            print_uint(regs->err_code);
        }
        terminal_putchar('\n');
    } else {
        terminal_write("[INTERRUPT] vector ");
        print_uint(regs->int_no);
        terminal_putchar('\n');
    }
}
