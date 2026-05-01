#include "io.h"
#include "pic.h"
#include <libc/stdint.h>

// Remap the master and slave PIC so IRQ0-IRQ15 use IDT vectors 32-47
void pic_remap(void){
    outb(0x20, 0x11);   // Send ICW1 to master PIC command port
    outb(0xA0, 0x11);   // Send ICW1 to slave PIC command port
    outb(0x21, 0x20);   // Send ICW2 to master PIC data port
    outb(0xA1, 0x28);   // Send ICW2 to slave PIC data port
    outb(0x21, 0x04);   // Send ICW3 to master PIC: tell it that slave PIC is connected on IRQ2
    outb(0xA1, 0x02);   // Send ICW3 to slave PIC: tell it its cascade identity is IRQ2
    outb(0x21, 0x01);   // Send ICW4 to master PIC
    outb(0xA1, 0x01);   // Send ICW4 to slave PIC
    outb(0x21, 0x00);   // Clear interrupt mask on master PIC
    outb(0xA1, 0x00);   // Clear interrupt mask on slave PIC
}

// Send end of interrupt (EOI) to the PIC after IRQ
void pic_send_eoi(uint8_t irq){
    if (irq >= 8){              // Check if interrupt came from slave PIC
        outb(0xA0, 0x20);       // Send EOI to slave PIC command port
    }   
    outb(0x20, 0x20);           // Send EOI to master PIC command port
}
