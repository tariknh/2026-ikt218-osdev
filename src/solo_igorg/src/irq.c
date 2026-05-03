#include <irq.h>
#include <idt.h>
#include <io.h>
#include <terminal.h>
#include <libc/stdint.h>
#include <keyboard.h>
#include <pit.h>

/*
 * 8259 PIC ports.
 * Master PIC handles IRQ0-IRQ7.
 * Slave PIC handles IRQ8-IRQ15.
 */
#define PIC1_COMMAND 0x20
#define PIC1_DATA    0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA    0xA1

#define PIC_EOI      0x20

#define ICW1_INIT    0x10
#define ICW1_ICW4    0x01
#define ICW4_8086    0x01

#define IDT_INTERRUPT_GATE 0x8E
#define KERNEL_CODE_SELECTOR 0x08

/*
 * IRQ assembly stubs implemented in irq.asm.
 */
extern void irq0(void);
extern void irq1(void);
extern void irq2(void);
extern void irq3(void);
extern void irq4(void);
extern void irq5(void);
extern void irq6(void);
extern void irq7(void);
extern void irq8(void);
extern void irq9(void);
extern void irq10(void);
extern void irq11(void);
extern void irq12(void);
extern void irq13(void);
extern void irq14(void);
extern void irq15(void);

/*
 * Remaps PIC so IRQ0-IRQ15 use interrupt vectors 0x20-0x2F.
 */
static void pic_remap(void)
{
    uint8_t master_mask = inb(PIC1_DATA);
    uint8_t slave_mask = inb(PIC2_DATA);

    /*
     * Initialization sequence.
     */
    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();
    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();

    /*
     * Set vector offsets:
     * Master PIC -> 0x20
     * Slave PIC  -> 0x28
     */
    outb(PIC1_DATA, 0x20);
    io_wait();
    outb(PIC2_DATA, 0x28);
    io_wait();

    /*
     * Tells master PIC that slave is connected on IRQ2.
     * Tells slave PIC its cascade identity.
     */
    outb(PIC1_DATA, 0x04);
    io_wait();
    outb(PIC2_DATA, 0x02);
    io_wait();

    /*
     * Uses 8086/88 mode.
     */
    outb(PIC1_DATA, ICW4_8086);
    io_wait();
    outb(PIC2_DATA, ICW4_8086);
    io_wait();

   /*
    * Enable IRQ0 and IRQ1 on the master PIC.
    * 0xFC = 11111100b
    * bit 0 = IRQ0 enabled
    * bit 1 = IRQ1 enabled
    * all other master IRQs disabled
    * 0xFF disables all IRQs on the slave PIC.
    */
    (void)master_mask;
    (void)slave_mask;

    outb(PIC1_DATA, 0xFC);
    outb(PIC2_DATA, 0xFF);
}

/*
 * Sends End-of-Interrupt signal to the PIC.
 * IRQ8-IRQ15 comes from slave PIC, so both slave and master must receive EOI.
 */
static void pic_send_eoi(uint32_t irq_number)
{
    if (irq_number >= 8) {
        outb(PIC2_COMMAND, PIC_EOI);
    }

    outb(PIC1_COMMAND, PIC_EOI);
}

/*
 * Registers IRQ0-IRQ15 in IDT.
 */
void irq_initialize(void)
{
    pic_remap();

    idt_set_gate(0x20, (uint32_t)irq0, KERNEL_CODE_SELECTOR, IDT_INTERRUPT_GATE);
    idt_set_gate(0x21, (uint32_t)irq1, KERNEL_CODE_SELECTOR, IDT_INTERRUPT_GATE);
    idt_set_gate(0x22, (uint32_t)irq2, KERNEL_CODE_SELECTOR, IDT_INTERRUPT_GATE);
    idt_set_gate(0x23, (uint32_t)irq3, KERNEL_CODE_SELECTOR, IDT_INTERRUPT_GATE);
    idt_set_gate(0x24, (uint32_t)irq4, KERNEL_CODE_SELECTOR, IDT_INTERRUPT_GATE);
    idt_set_gate(0x25, (uint32_t)irq5, KERNEL_CODE_SELECTOR, IDT_INTERRUPT_GATE);
    idt_set_gate(0x26, (uint32_t)irq6, KERNEL_CODE_SELECTOR, IDT_INTERRUPT_GATE);
    idt_set_gate(0x27, (uint32_t)irq7, KERNEL_CODE_SELECTOR, IDT_INTERRUPT_GATE);
    idt_set_gate(0x28, (uint32_t)irq8, KERNEL_CODE_SELECTOR, IDT_INTERRUPT_GATE);
    idt_set_gate(0x29, (uint32_t)irq9, KERNEL_CODE_SELECTOR, IDT_INTERRUPT_GATE);
    idt_set_gate(0x2A, (uint32_t)irq10, KERNEL_CODE_SELECTOR, IDT_INTERRUPT_GATE);
    idt_set_gate(0x2B, (uint32_t)irq11, KERNEL_CODE_SELECTOR, IDT_INTERRUPT_GATE);
    idt_set_gate(0x2C, (uint32_t)irq12, KERNEL_CODE_SELECTOR, IDT_INTERRUPT_GATE);
    idt_set_gate(0x2D, (uint32_t)irq13, KERNEL_CODE_SELECTOR, IDT_INTERRUPT_GATE);
    idt_set_gate(0x2E, (uint32_t)irq14, KERNEL_CODE_SELECTOR, IDT_INTERRUPT_GATE);
    idt_set_gate(0x2F, (uint32_t)irq15, KERNEL_CODE_SELECTOR, IDT_INTERRUPT_GATE);
}

void irq_handler(uint32_t irq_number)
{
    /*
     * IRQ0 - PIT/system timer.
     */
    if (irq_number == 0) {
        pit_handle_irq();
        pic_send_eoi(irq_number);
        return;
    }

    /*
     * IRQ1 - PS/2 keyboard interrupt.
     */
    if (irq_number == 1) {
        keyboard_handle_irq();
        pic_send_eoi(irq_number);
        return;
    }

    terminal_write("IRQ triggered: ");

    if (irq_number < 10) {
        terminal_putchar('0' + irq_number);
    } else {
        terminal_putchar('1');
        terminal_putchar('0' + (irq_number - 10));
    }

    terminal_putchar('\n');

    pic_send_eoi(irq_number);
}
