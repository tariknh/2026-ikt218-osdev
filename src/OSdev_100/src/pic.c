#include "../include/pic.h"

// sends a byte to a hardware port
void outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

// reads a byte from a hardware port
uint8_t inb(uint16_t port) {
    uint8_t value;
    __asm__ volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

// small delay so the PIC has time to process commands
void io_wait(void) {
    outb(0x80, 0);
}

// sets up the PIC so IRQs dont crash with CPU exceptions
// by default IRQs 0-7 overlap with CPU exceptions 8-15 which is bad
// so we move them to start at 32 instead
void pic_init(void) {
    // save the current masks so we can restore them later
    uint8_t mask1 = inb(PIC1_DATA);
    uint8_t mask2 = inb(PIC2_DATA);

    // tell both PICs we are going to reconfigure them
    outb(PIC1_COMMAND, 0x11); io_wait();
    outb(PIC2_COMMAND, 0x11); io_wait();

    // move master PIC IRQs to start at interrupt 32
    outb(PIC1_DATA, PIC1_OFFSET); io_wait();
    // move slave PIC IRQs to start at interrupt 40
    outb(PIC2_DATA, PIC2_OFFSET); io_wait();

    // tell master PIC that slave is connected at IRQ2
    outb(PIC1_DATA, 0x04); io_wait();
    // tell slave PIC its own ID
    outb(PIC2_DATA, 0x02); io_wait();

    // set both PICs to 8086 mode
    outb(PIC1_DATA, 0x01); io_wait();
    outb(PIC2_DATA, 0x01); io_wait();

    // put the old masks back
    outb(PIC1_DATA, mask1);
    outb(PIC2_DATA, mask2);
}

// tells the PIC we are done handling an interrupt
// if the IRQ came from the slave PIC we need to notify both
void pic_send_eoi(uint8_t irq) {
    if (irq >= 8) {
        outb(PIC2_COMMAND, PIC_EOI);
    }
    outb(PIC1_COMMAND, PIC_EOI);
}