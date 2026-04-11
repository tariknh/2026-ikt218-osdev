#include "interrupts.h"
#include "../pit/pit.h"
#include "../vga_text_mode_interface/vga_text_mode_interface.h"

static struct idt_gate idt[256];
static struct idt_pointer idtp;

/** Loads the interrupt descriptor table */
void load_idt(struct idt_pointer idt_pointer) {
    // __asm__ __volatile__ (".intel_syntax noprefix\r\n");
    __asm__ __volatile__ ("lidt %0\r\n" : : "m"(idt_pointer));
    // __asm__ __volatile__ (".att_syntax prefix\r\n");
}

struct idt_gate create_idt_gate(uint32_t offset, uint16_t selector, uint8_t attributes)
{
    struct idt_gate a;
    a.low_offset = (offset);
    a.hi_offset = (offset >> 16);
    a.selector = selector;
    a.attributes = attributes;
    a.RESERVED_DO_NOT_USE = 0;

    return a;
}

uint8_t create_idt_attributes(bool present, int8_t ring, uint8_t type) {
    uint8_t a = 0;
    if (present) { a = 0b10000000; }
    else if (!present) { a = 0; }
    a = a | (ring << 5);
    a = a | type;
    return a;
}

// Helper to write to I/O ports
static void outb(uint16_t port, uint8_t val) {
    __asm__ __volatile__ ( "outb %0, %1\r\n" : : "a"(val), "dN"(port) );
}

static uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ __volatile__ ( "inb %1, %0" : "=a"(ret) : "Nd"(port) );
    return ret;
}

/** Helper to add a tiny delay for older hardware compatibility 
 * \todo Is this needed?
*/
static void io_wait() {
    outb(0x80, 0);
}

void pic_remap(int offset1, int offset2) {

    // 2. Start initialization sequence (ICW1)
    outb(0x20, 0x11); 
    io_wait();
    outb(0xA0, 0x11);
    io_wait();

    // 3. Set Vector Offsets (ICW2)
    outb(0x21, offset1); // Usually 0x20 (32)
    io_wait();
    outb(0xA1, offset2); // Usually 0x28 (40)
    io_wait();

    // 4. Tell Master PIC there is a slave PIC at IRQ2 (ICW3)
    outb(0x21, 0x04);
    io_wait();
    // 5. Tell Slave PIC its cascade identity (ICW3)
    outb(0xA1, 0x02);
    io_wait();

    // 6. Set mode to 8086/88 (ICW4)
    outb(0x21, 0x01);
    io_wait();
    outb(0xA1, 0x01);
    io_wait();

    // 7. Restore masks (or leave at 0 to enable all)
    outb(0x21, 0);
    outb(0xA1, 0);
}

void init_idt() {
    pic_remap(0x20, 0x28);

    // Port 0x21 is the Master PIC data port. 
    // Bit 0 corresponds to IRQ0 (PIT) and bit 1 to IRQ1 (keyboard).
    outb(0x21, 0xFC);
    outb(0xA1, 0xFF);

    idtp.offset = (uint32_t)idt;
    idtp.size = (256*8)-1;
    idt[32] = create_idt_gate((uint32_t)pit_irq_handler, 0x08, create_idt_attributes(true, 0, idt_type_interrupt));
    idt[33] = create_idt_gate((uint32_t)keyboard_interrupt_handler, 0x08, create_idt_attributes(true, 0, idt_type_interrupt));
    load_idt(idtp);
    __asm__ __volatile__ ("sti");
}

__attribute__((interrupt))
__attribute__((target("general-regs-only")))
void keyboard_interrupt_handler(struct interrupt_frame* frame) {
    keyboard_callback();
}

// This code is too HEAVY to be inside the keyboard_interupt_handler()
void keyboard_callback(){
    uint8_t scan = inb(0x60);
    struct VgaTextModeInterface screen = NewVgaTextModeInterface();
    char s[2] = {scan, 0};
    screen.Print(&screen, s, VgaColor(vga_white, vga_black));
    outb(0x20, 0x20); // Send EOI
}
