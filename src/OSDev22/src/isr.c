#include "isr.h"
#include "idt.h"
#include "terminal.h"

/*
 * Navn på alle 32 CPU-exceptions.
 * Brukes for å skrive ut en lesbar melding når noe går galt.
 * Hentet fra Intel Software Developer Manual, Volume 3, Chapter 6.
 */
static const char* exception_names[32] = {
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Overflow",
    "Bound Range Exceeded",
    "Invalid Opcode",
    "Device Not Available",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Invalid TSS",
    "Segment Not Present",
    "Stack-Segment Fault",
    "General Protection Fault",
    "Page Fault",
    "Reserved",
    "x87 Floating-Point Exception",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating-Point Exception",
    "Virtualization Exception",
    "Control Protection Exception",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Hypervisor Injection Exception",
    "VMM Communication Exception",
    "Security Exception",
    "Reserved"
};

/* Alle 32 assembly-stubbene (definert i isr.asm) */
extern void isr0(void);
extern void isr1(void);
extern void isr2(void);
extern void isr3(void);
extern void isr4(void);
extern void isr5(void);
extern void isr6(void);
extern void isr7(void);
extern void isr8(void);
extern void isr9(void);
extern void isr10(void);
extern void isr11(void);
extern void isr12(void);
extern void isr13(void);
extern void isr14(void);
extern void isr15(void);
extern void isr16(void);
extern void isr17(void);
extern void isr18(void);
extern void isr19(void);
extern void isr20(void);
extern void isr21(void);
extern void isr22(void);
extern void isr23(void);
extern void isr24(void);
extern void isr25(void);
extern void isr26(void);
extern void isr27(void);
extern void isr28(void);
extern void isr29(void);
extern void isr30(void);
extern void isr31(void);

/*
 * Registrerer alle 32 CPU-exceptions i IDT-en.
 * Hver entry peker til sin assembly-stub, som til slutt
 * kaller isr_handler() i C.
 */
void isr_init(void)
{
    idt_set_entry(0,  (uint32_t)isr0,  0x08, IDT_FLAG_KERNEL);
    idt_set_entry(1,  (uint32_t)isr1,  0x08, IDT_FLAG_KERNEL);
    idt_set_entry(2,  (uint32_t)isr2,  0x08, IDT_FLAG_KERNEL);
    idt_set_entry(3,  (uint32_t)isr3,  0x08, IDT_FLAG_KERNEL);
    idt_set_entry(4,  (uint32_t)isr4,  0x08, IDT_FLAG_KERNEL);
    idt_set_entry(5,  (uint32_t)isr5,  0x08, IDT_FLAG_KERNEL);
    idt_set_entry(6,  (uint32_t)isr6,  0x08, IDT_FLAG_KERNEL);
    idt_set_entry(7,  (uint32_t)isr7,  0x08, IDT_FLAG_KERNEL);
    idt_set_entry(8,  (uint32_t)isr8,  0x08, IDT_FLAG_KERNEL);
    idt_set_entry(9,  (uint32_t)isr9,  0x08, IDT_FLAG_KERNEL);
    idt_set_entry(10, (uint32_t)isr10, 0x08, IDT_FLAG_KERNEL);
    idt_set_entry(11, (uint32_t)isr11, 0x08, IDT_FLAG_KERNEL);
    idt_set_entry(12, (uint32_t)isr12, 0x08, IDT_FLAG_KERNEL);
    idt_set_entry(13, (uint32_t)isr13, 0x08, IDT_FLAG_KERNEL);
    idt_set_entry(14, (uint32_t)isr14, 0x08, IDT_FLAG_KERNEL);
    idt_set_entry(15, (uint32_t)isr15, 0x08, IDT_FLAG_KERNEL);
    idt_set_entry(16, (uint32_t)isr16, 0x08, IDT_FLAG_KERNEL);
    idt_set_entry(17, (uint32_t)isr17, 0x08, IDT_FLAG_KERNEL);
    idt_set_entry(18, (uint32_t)isr18, 0x08, IDT_FLAG_KERNEL);
    idt_set_entry(19, (uint32_t)isr19, 0x08, IDT_FLAG_KERNEL);
    idt_set_entry(20, (uint32_t)isr20, 0x08, IDT_FLAG_KERNEL);
    idt_set_entry(21, (uint32_t)isr21, 0x08, IDT_FLAG_KERNEL);
    idt_set_entry(22, (uint32_t)isr22, 0x08, IDT_FLAG_KERNEL);
    idt_set_entry(23, (uint32_t)isr23, 0x08, IDT_FLAG_KERNEL);
    idt_set_entry(24, (uint32_t)isr24, 0x08, IDT_FLAG_KERNEL);
    idt_set_entry(25, (uint32_t)isr25, 0x08, IDT_FLAG_KERNEL);
    idt_set_entry(26, (uint32_t)isr26, 0x08, IDT_FLAG_KERNEL);
    idt_set_entry(27, (uint32_t)isr27, 0x08, IDT_FLAG_KERNEL);
    idt_set_entry(28, (uint32_t)isr28, 0x08, IDT_FLAG_KERNEL);
    idt_set_entry(29, (uint32_t)isr29, 0x08, IDT_FLAG_KERNEL);
    idt_set_entry(30, (uint32_t)isr30, 0x08, IDT_FLAG_KERNEL);
    idt_set_entry(31, (uint32_t)isr31, 0x08, IDT_FLAG_KERNEL);
}

/*
 * Felles handler for alle CPU-exceptions.
 * Kalles fra isr_common i assembly etter at alle registre er lagret.
 * Skriver ut navnet på exceptionen og stopper maskinen hvis det
 * er noe alvorlig (som page fault eller double fault).
 */
void isr_handler(struct isr_frame* frame)
{
    /* Sørg for at vi ikke går utenfor tabellen */
    if (frame->int_no < 32) {
        printf("\n[EXCEPTION] %s (int %u, err=0x%x)\n",
               exception_names[frame->int_no],
               frame->int_no,
               frame->err_code);
    }
    /* Halt the CPU */
    for (;;) {
        __asm__ volatile("hlt");
    }
}