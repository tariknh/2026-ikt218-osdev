#include <interrupts.h>
#include <common.h>

static struct int_handler_t irq_handlers[16];

void init_irq(void)
{
    for (int i = 0; i < 16; i++) {
        irq_handlers[i].handler = 0;
        irq_handlers[i].data = 0;
    }

    outb(0x21, 0x00); 
    outb(0xA1, 0x00); 
}


void register_irq_handler(int irq, isr_t handler, void* ctx)
{
    if (irq < 0 || irq > 15) return;
    irq_handlers[irq].handler = handler;
    irq_handlers[irq].data = ctx;
}

void irq_handler(registers_t* regs)
{
    int irq = (int)(regs->int_no & 0xFF) - IRQ_BASE;

    if (irq >= 0 && irq <= 15) {
        if (irq_handlers[irq].handler) {
            irq_handlers[irq].handler(regs, irq_handlers[irq].data);
        }
    }

    if ((regs->int_no & 0xFF) >= 40) outb(0xA0, 0x20);
    outb(0x20, 0x20);
}
