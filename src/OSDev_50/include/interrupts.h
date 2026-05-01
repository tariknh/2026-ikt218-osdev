#ifndef INTERRUPTS_H
#define INTERRUPTS_H
#pragma once
#include <libc/stdint.h>

#define IRQ_BASE 32

typedef struct registers
{
    uint32_t ds;
    uint32_t edi, esi, ebp, useless_value, ebx, edx, ecx, eax;
    uint32_t int_no, err_code;
    uint32_t eip, cs, eflags, esp, ss;
} registers_t;

typedef void (*isr_t)(registers_t* regs, void* ctx);

struct int_handler_t {
    isr_t handler;
    void* data;
};

void register_interrupt_handler(uint8_t n, isr_t handler, void* ctx);
void register_irq_handler(int irq, isr_t handler, void* ctx);

void init_irq(void);

void isr_handler(registers_t* regs);
void irq_handler(registers_t* regs);

void isr_register_default_prints(void);

#endif
