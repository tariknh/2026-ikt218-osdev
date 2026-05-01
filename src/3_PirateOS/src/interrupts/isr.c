#include "interrupts/isr.h"
#include "descriptor_tables/idt.h"
#include "libc/stdio.h"
#include "arch/i386/io.h"



isr_t interrupt_handlers[256]; // Array of function pointers to hold custom interrupt handlers

// This allows us to register a custom handler for a specific interrupt number.
void register_interrupt_handler(uint8_t n, isr_t handler)
{
    interrupt_handlers[n] = handler;
}

// Exception messages
const char *exception_messages[] = {
    "Division Error",                 // 0  #DE
    "Debug",                          // 1  #DB
    "Non-Maskable Interrupt",         // 2  -
    "Breakpoint",                     // 3  #BP
    "Overflow",                       // 4  #OF
    "Bound Range Exceeded",           // 5  #BR
    "Invalid Opcode",                 // 6  #UD
    "Device Not Available",           // 7  #NM
    "Double Fault",                   // 8  #DF
    "Coprocessor Segment Overrun",    // 9  -
    "Invalid TSS",                    // 10 #TS
    "Segment Not Present",            // 11 #NP
    "Stack-Segment Fault",            // 12 #SS
    "General Protection Fault",       // 13 #GP
    "Page Fault",                     // 14 #PF
    "Reserved",                       // 15 -
    "x87 Floating-Point Exception",   // 16 #MF
    "Alignment Check",                // 17 #AC
    "Machine Check",                  // 18 #MC
    "SIMD Floating-Point Exception",  // 19 #XM/#XF
    "Virtualization Exception",       // 20 #VE
    "Control Protection Exception",   // 21 #CP
    "Reserved",                       // 22 -
    "Reserved",                       // 23 -
    "Reserved",                       // 24 -
    "Reserved",                       // 25 -
    "Reserved",                       // 26 -
    "Reserved",                       // 27 -
    "Hypervisor Injection Exception", // 28 #HV
    "VMM Communication Exception",    // 29 #VC
    "Security Exception",             // 30 #SX
    "Reserved"                        // 31 -
};

void init_isr()
{ // Set up the IDT entries for the first 32 ISRs (CPU exceptions)
    idt_set_gate(0, (uint32_t)isr0, 0x08, 0x8E);
    idt_set_gate(1, (uint32_t)isr1, 0x08, 0x8E);
    idt_set_gate(2, (uint32_t)isr2, 0x08, 0x8E);
    idt_set_gate(3, (uint32_t)isr3, 0x08, 0x8E);
    idt_set_gate(4, (uint32_t)isr4, 0x08, 0x8E);
    idt_set_gate(5, (uint32_t)isr5, 0x08, 0x8E);
    idt_set_gate(6, (uint32_t)isr6, 0x08, 0x8E);
    idt_set_gate(7, (uint32_t)isr7, 0x08, 0x8E);
    idt_set_gate(8, (uint32_t)isr8, 0x08, 0x8E);
    idt_set_gate(9, (uint32_t)isr9, 0x08, 0x8E);
    idt_set_gate(10, (uint32_t)isr10, 0x08, 0x8E);
    idt_set_gate(11, (uint32_t)isr11, 0x08, 0x8E);
    idt_set_gate(12, (uint32_t)isr12, 0x08, 0x8E);
    idt_set_gate(13, (uint32_t)isr13, 0x08, 0x8E);
    idt_set_gate(14, (uint32_t)isr14, 0x08, 0x8E);
    idt_set_gate(15, (uint32_t)isr15, 0x08, 0x8E);
    idt_set_gate(16, (uint32_t)isr16, 0x08, 0x8E);
    idt_set_gate(17, (uint32_t)isr17, 0x08, 0x8E);
    idt_set_gate(18, (uint32_t)isr18, 0x08, 0x8E);
    idt_set_gate(19, (uint32_t)isr19, 0x08, 0x8E);
    idt_set_gate(20, (uint32_t)isr20, 0x08, 0x8E);
    idt_set_gate(21, (uint32_t)isr21, 0x08, 0x8E);
    idt_set_gate(22, (uint32_t)isr22, 0x08, 0x8E);
    idt_set_gate(23, (uint32_t)isr23, 0x08, 0x8E);
    idt_set_gate(24, (uint32_t)isr24, 0x08, 0x8E);
    idt_set_gate(25, (uint32_t)isr25, 0x08, 0x8E);
    idt_set_gate(26, (uint32_t)isr26, 0x08, 0x8E);
    idt_set_gate(27, (uint32_t)isr27, 0x08, 0x8E);
    idt_set_gate(28, (uint32_t)isr28, 0x08, 0x8E);
    idt_set_gate(29, (uint32_t)isr29, 0x08, 0x8E);
    idt_set_gate(30, (uint32_t)isr30, 0x08, 0x8E);
    idt_set_gate(31, (uint32_t)isr31, 0x08, 0x8E);
}

void init_irq()
{ // Set up the IDT entries for IRQs (hardware interrupts)
    // Remap PIC
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    outb(0x21, 0x20); // Master PIC command offset to IRQ0 - IRQ7 = 32 - 39
    outb(0xA1, 0x28); // Slave PIC command offset to IRQ8 - IRQ15 = 40 - 47
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    outb(0x21, 0x0);
    outb(0xA1, 0x0);

    idt_set_gate(32, (uint32_t)irq0, 0x08, 0x8E);
    idt_set_gate(33, (uint32_t)irq1, 0x08, 0x8E);
    idt_set_gate(34, (uint32_t)irq2, 0x08, 0x8E);
    idt_set_gate(35, (uint32_t)irq3, 0x08, 0x8E);
    idt_set_gate(36, (uint32_t)irq4, 0x08, 0x8E);
    idt_set_gate(37, (uint32_t)irq5, 0x08, 0x8E);
    idt_set_gate(38, (uint32_t)irq6, 0x08, 0x8E);
    idt_set_gate(39, (uint32_t)irq7, 0x08, 0x8E);
    idt_set_gate(40, (uint32_t)irq8, 0x08, 0x8E);
    idt_set_gate(41, (uint32_t)irq9, 0x08, 0x8E);
    idt_set_gate(42, (uint32_t)irq10, 0x08, 0x8E);
    idt_set_gate(43, (uint32_t)irq11, 0x08, 0x8E);
    idt_set_gate(44, (uint32_t)irq12, 0x08, 0x8E);
    idt_set_gate(45, (uint32_t)irq13, 0x08, 0x8E);
    idt_set_gate(46, (uint32_t)irq14, 0x08, 0x8E);
    idt_set_gate(47, (uint32_t)irq15, 0x08, 0x8E);
}

// This gets called from our ASM interrupt handler stub.
void isr_handler(registers_t *regs)
{
    if (interrupt_handlers[regs->int_no] != 0)
    {                                                     // If a custom handler is registered for this interrupt, call it
        isr_t handler = interrupt_handlers[regs->int_no]; // Get the handler from the array
        handler(regs);                                    // Call the handler with the register state
    }
    else
    {
        printf("Received interrupt: %d", regs->int_no); // Print the interrupt number
        if (regs->int_no < 32)
        {                                                        // If it's a CPU exception, also print the corresponding message
            printf(" (%s)\n", exception_messages[regs->int_no]); // Print the exception message for CPU exceptions
        }
        else
        {
            printf("\n"); // Print a newline for non-CPU exceptions
        }
    }
}

// This gets called from our ASM IRQ handler stub.
void irq_handler(registers_t *regs)
{
    // Send an EOI (end of interrupt) signal to the PICs.
    if (regs->int_no >= 40)
    {
        // Send reset signal to slave.
        outb(0xA0, 0x20);
    }
    // Send reset signal to master. (As well as slave, if necessary).
    outb(0x20, 0x20);

    if (interrupt_handlers[regs->int_no] != 0)
    {                                                     // If a custom handler is registered for this interrupt, call it
        isr_t handler = interrupt_handlers[regs->int_no]; // Get the handler from the array
        handler(regs);                                    // Call the handler with the register state
    }
}
