#include <libc/stdint.h>
#include <pic.h>

void pic_remap(void){
    // Send ICW1 to both PICs
    outb(0x20, 0x11); // Master PIC command port
    outb(0xA0, 0x11); // Slave PIC command port

    // Send ICW2: remap offset address of IDT
    outb(0x21, 0x20); // Master PIC data port: remap to 0x20-0x27
    outb(0xA1, 0x28); // Slave PIC data port: remap to 0x28-0x2F

    // Send ICW3: setup cascading
    outb(0x21, 0x04); // Master PIC data port: tell master that there is a slave at IRQ2 (0000 0100)
    outb(0xA1, 0x02); // Slave PIC data port: tell slave its cascade identity (0000 0010)

    // Send ICW4: environment info
    outb(0x21, 0x01);
    outb(0xA1, 0x01);

    // Mask all interrupts
    outb(0x21, 0xFF);
    outb(0xA1, 0xFF);
}

void pic_mask_all(void){
    outb(0x21, 0xFF); // Mask all interrupts on Master PIC
    outb(0xA1, 0xFF); // Mask all interrupts on Slave PIC
}

