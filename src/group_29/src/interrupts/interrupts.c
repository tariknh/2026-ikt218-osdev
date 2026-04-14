#include "interrupts.h"

#include "../keyboard/keyboard.h"
#include "../pit/pit.h"
#include "../printing/printing.h"

#define PIC1_COMMAND_PORT 0x20
#define PIC1_DATA_PORT 0x21
#define PIC2_COMMAND_PORT 0xA0
#define PIC2_DATA_PORT 0xA1
#define PIC_EOI_COMMAND 0x20

static struct idt_gate idt[256];
static struct idt_pointer idtp;

static void irq2_handler(struct interrupt_frame* frame);
static void irq3_handler(struct interrupt_frame* frame);
static void irq4_handler(struct interrupt_frame* frame);
static void irq5_handler(struct interrupt_frame* frame);
static void irq6_handler(struct interrupt_frame* frame);
static void irq7_handler(struct interrupt_frame* frame);
static void irq8_handler(struct interrupt_frame* frame);
static void irq9_handler(struct interrupt_frame* frame);
static void irq10_handler(struct interrupt_frame* frame);
static void irq11_handler(struct interrupt_frame* frame);
static void irq12_handler(struct interrupt_frame* frame);
static void irq13_handler(struct interrupt_frame* frame);
static void irq14_handler(struct interrupt_frame* frame);
static void irq15_handler(struct interrupt_frame* frame);

static void isr0_handler(struct interrupt_frame* frame);
static void isr1_handler(struct interrupt_frame* frame);
static void isr2_handler(struct interrupt_frame* frame);

/** Loads the interrupt descriptor table */
void load_idt(struct idt_pointer idt_pointer) {
    __asm__ __volatile__("lidt %0\r\n" : : "m"(idt_pointer));
}

struct idt_gate create_idt_gate(uint32_t offset, uint16_t selector, uint8_t attributes)
{
    struct idt_gate gate;
    gate.low_offset = (uint16_t)offset;
    gate.hi_offset = (uint16_t)(offset >> 16);
    gate.selector = selector;
    gate.attributes = attributes;
    gate.RESERVED_DO_NOT_USE = 0;

    return gate;
}

uint8_t create_idt_attributes(bool present, int8_t ring, uint8_t type) {
    uint8_t attributes = 0;
    if (present) {
        attributes = 0b10000000;
    }

    attributes = (uint8_t)(attributes | (uint8_t)(ring << 5));
    attributes = (uint8_t)(attributes | type);
    return attributes;
}

static void outb(uint16_t port, uint8_t val) {
    __asm__ __volatile__("outb %0, %1\r\n" : : "a"(val), "dN"(port));
}

static uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ __volatile__("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static void io_wait(void) {
    outb(0x80, 0);
}

static void send_eoi(uint8_t irq_number)
{
    if (irq_number >= 8U) {
        outb(PIC2_COMMAND_PORT, PIC_EOI_COMMAND);
    }

    outb(PIC1_COMMAND_PORT, PIC_EOI_COMMAND);
}

static void print_interrupt_message(const char* message, uint8_t color)
{
    print(message, color);
}

static void handle_generic_irq(uint8_t irq_number, const char* message)
{
    print_interrupt_message(message, VgaColor(vga_black, vga_light_cyan));
    send_eoi(irq_number);
}

#define DEFINE_IRQ_HANDLER(NAME, IRQ_NUMBER, MESSAGE) \
    __attribute__((interrupt)) \
    __attribute__((target("general-regs-only"))) \
    static void NAME(struct interrupt_frame* frame) { \
        (void)frame; \
        handle_generic_irq(IRQ_NUMBER, MESSAGE); \
    }

__attribute__((interrupt))
__attribute__((target("general-regs-only")))
static void isr0_handler(struct interrupt_frame* frame)
{
    (void)frame;
    print_interrupt_message("ISR 0 triggered by software interrupt 0x0\n", VgaColor(vga_black, vga_yellow));
}

__attribute__((interrupt))
__attribute__((target("general-regs-only")))
static void isr1_handler(struct interrupt_frame* frame)
{
    (void)frame;
    print_interrupt_message("ISR 1 triggered by software interrupt 0x1\n", VgaColor(vga_black, vga_yellow));
}

__attribute__((interrupt))
__attribute__((target("general-regs-only")))
static void isr2_handler(struct interrupt_frame* frame)
{
    (void)frame;
    print_interrupt_message("ISR 2 triggered by software interrupt 0x2\n", VgaColor(vga_black, vga_yellow));
}

DEFINE_IRQ_HANDLER(irq2_handler, 2, "IRQ2 triggered\n");
DEFINE_IRQ_HANDLER(irq3_handler, 3, "IRQ3 triggered\n");
DEFINE_IRQ_HANDLER(irq4_handler, 4, "IRQ4 triggered\n");
DEFINE_IRQ_HANDLER(irq5_handler, 5, "IRQ5 triggered\n");
DEFINE_IRQ_HANDLER(irq6_handler, 6, "IRQ6 triggered\n");
DEFINE_IRQ_HANDLER(irq7_handler, 7, "IRQ7 triggered\n");
DEFINE_IRQ_HANDLER(irq8_handler, 8, "IRQ8 triggered\n");
DEFINE_IRQ_HANDLER(irq9_handler, 9, "IRQ9 triggered\n");
DEFINE_IRQ_HANDLER(irq10_handler, 10, "IRQ10 triggered\n");
DEFINE_IRQ_HANDLER(irq11_handler, 11, "IRQ11 triggered\n");
DEFINE_IRQ_HANDLER(irq12_handler, 12, "IRQ12 triggered\n");
DEFINE_IRQ_HANDLER(irq13_handler, 13, "IRQ13 triggered\n");
DEFINE_IRQ_HANDLER(irq14_handler, 14, "IRQ14 triggered\n");
DEFINE_IRQ_HANDLER(irq15_handler, 15, "IRQ15 triggered\n");

void pic_remap(int offset1, int offset2) {
    outb(PIC1_COMMAND_PORT, 0x11);
    io_wait();
    outb(PIC2_COMMAND_PORT, 0x11);
    io_wait();

    outb(PIC1_DATA_PORT, (uint8_t)offset1);
    io_wait();
    outb(PIC2_DATA_PORT, (uint8_t)offset2);
    io_wait();

    outb(PIC1_DATA_PORT, 0x04);
    io_wait();
    outb(PIC2_DATA_PORT, 0x02);
    io_wait();

    outb(PIC1_DATA_PORT, 0x01);
    io_wait();
    outb(PIC2_DATA_PORT, 0x01);
    io_wait();

    outb(PIC1_DATA_PORT, 0xFC);
    outb(PIC2_DATA_PORT, 0xFF);
}

void init_idt() {
    pic_remap(0x20, 0x28);

    idtp.offset = (uint32_t)idt;
    idtp.size = (uint16_t)((256 * 8) - 1);

    idt[0] = create_idt_gate((uint32_t)isr0_handler, 0x08, create_idt_attributes(true, 0, idt_type_interrupt));
    idt[1] = create_idt_gate((uint32_t)isr1_handler, 0x08, create_idt_attributes(true, 0, idt_type_interrupt));
    idt[2] = create_idt_gate((uint32_t)isr2_handler, 0x08, create_idt_attributes(true, 0, idt_type_interrupt));

    idt[32] = create_idt_gate((uint32_t)pit_irq_handler, 0x08, create_idt_attributes(true, 0, idt_type_interrupt));
    idt[33] = create_idt_gate((uint32_t)keyboard_interrupt_handler, 0x08, create_idt_attributes(true, 0, idt_type_interrupt));
    idt[34] = create_idt_gate((uint32_t)irq2_handler, 0x08, create_idt_attributes(true, 0, idt_type_interrupt));
    idt[35] = create_idt_gate((uint32_t)irq3_handler, 0x08, create_idt_attributes(true, 0, idt_type_interrupt));
    idt[36] = create_idt_gate((uint32_t)irq4_handler, 0x08, create_idt_attributes(true, 0, idt_type_interrupt));
    idt[37] = create_idt_gate((uint32_t)irq5_handler, 0x08, create_idt_attributes(true, 0, idt_type_interrupt));
    idt[38] = create_idt_gate((uint32_t)irq6_handler, 0x08, create_idt_attributes(true, 0, idt_type_interrupt));
    idt[39] = create_idt_gate((uint32_t)irq7_handler, 0x08, create_idt_attributes(true, 0, idt_type_interrupt));
    idt[40] = create_idt_gate((uint32_t)irq8_handler, 0x08, create_idt_attributes(true, 0, idt_type_interrupt));
    idt[41] = create_idt_gate((uint32_t)irq9_handler, 0x08, create_idt_attributes(true, 0, idt_type_interrupt));
    idt[42] = create_idt_gate((uint32_t)irq10_handler, 0x08, create_idt_attributes(true, 0, idt_type_interrupt));
    idt[43] = create_idt_gate((uint32_t)irq11_handler, 0x08, create_idt_attributes(true, 0, idt_type_interrupt));
    idt[44] = create_idt_gate((uint32_t)irq12_handler, 0x08, create_idt_attributes(true, 0, idt_type_interrupt));
    idt[45] = create_idt_gate((uint32_t)irq13_handler, 0x08, create_idt_attributes(true, 0, idt_type_interrupt));
    idt[46] = create_idt_gate((uint32_t)irq14_handler, 0x08, create_idt_attributes(true, 0, idt_type_interrupt));
    idt[47] = create_idt_gate((uint32_t)irq15_handler, 0x08, create_idt_attributes(true, 0, idt_type_interrupt));

    load_idt(idtp);
    __asm__ __volatile__("sti");
}

void trigger_isr_demo(void)
{
    __asm__ __volatile__("int $0x0");
    __asm__ __volatile__("int $0x1");
    __asm__ __volatile__("int $0x2");
}

__attribute__((interrupt))
__attribute__((target("general-regs-only")))
void keyboard_interrupt_handler(struct interrupt_frame* frame) {
    (void)frame;
    keyboard_callback();
}

void keyboard_callback(){
    uint8_t scan = inb(0x60);
    keyboard_handle_scancode(scan);
    send_eoi(1);
}
