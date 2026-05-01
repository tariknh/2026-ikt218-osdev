#include <interrupts.h>
#include <terminal.h>

static struct int_handler_t int_handlers[256];

void register_interrupt_handler(uint8_t n, isr_t handler, void* ctx)
{
    int_handlers[n].handler = handler;
    int_handlers[n].data = ctx;
}

static void isr_print_0(registers_t* regs, void* ctx)
{
    (void)regs; (void)ctx;
    terminal_write("ISR 0 triggered\n");
}

static void isr_print_3(registers_t* regs, void* ctx)
{
    (void)regs; (void)ctx;
    terminal_write("ISR 3 triggered\n");
}

static void isr_print_4(registers_t* regs, void* ctx)
{
    (void)regs; (void)ctx;
    terminal_write("ISR 4 triggered\n");
}

void isr_register_default_prints(void)
{
    register_interrupt_handler(0, isr_print_0, 0);
    register_interrupt_handler(3, isr_print_3, 0);
    register_interrupt_handler(4, isr_print_4, 0);
}

void isr_handler(registers_t* regs)
{
    uint8_t n = (uint8_t)(regs->int_no & 0xFF);
    if (int_handlers[n].handler) {
        int_handlers[n].handler(regs, int_handlers[n].data);
    }
}
