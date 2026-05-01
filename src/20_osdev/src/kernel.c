#include <libc/stdint.h>
#include "gdt.h"
#include "idt.h"
#include "isr.h"
#include "irq.h"

static volatile uint16_t *video = (volatile uint16_t *)0xB8000;

static void print(const char *text)
{
    for (int i = 0; text[i] != '\0'; i++)
    {
        video[i] = (uint16_t)text[i] | (uint16_t)0x0F00;
    }
}

void main(void)
{
    gdt_init();
    idt_init();
    isr_init();
    irq_init();

    print("GDT, IDT, ISR and IRQ loaded");

    __asm__ volatile("int $0x0");
    __asm__ volatile("int $0x1");
    __asm__ volatile("int $0x2");

    while (1)
    {
        __asm__ volatile("hlt");
    }
}