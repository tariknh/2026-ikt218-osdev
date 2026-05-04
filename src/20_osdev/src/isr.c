#include <libc/stdint.h>
#include "isr.h"
#include "idt.h"

extern void isr0(void);
extern void isr1(void);
extern void isr2(void);

static volatile uint16_t *video = (volatile uint16_t *)0xB8000;

static void print_at(const char *text, int row)
{
    int offset = row * 80;

    for (int i = 0; text[i] != '\0'; i++)
    {
        video[offset + i] = (uint16_t)text[i] | (uint16_t)0x0F00;
    }
}

void isr0_handler(void)
{
    printf("ISR 0 triggered\n");
}

void isr1_handler(void)
{
    printf("ISR 1 triggered\n");
}

void isr2_handler(void)
{
    printf("ISR 2 triggered\n");
}

void isr_init(void)
{
    idt_set_gate(0, (uint32_t)isr0);
    idt_set_gate(1, (uint32_t)isr1);
    idt_set_gate(2, (uint32_t)isr2);
}