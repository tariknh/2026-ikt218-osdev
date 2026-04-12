#include "irq.h"
#include "terminal.h"
#include "pit.h"


//master and slave PIC port addresses
#define PIC1_COMMAND 0x20
#define PIC1_DATA 0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA 0xA1
#define PIC_EOI 0x20 //end of interrupt command code

//initialisation command words
#define ICW1_INIT 0x10 //initialisation required
#define ICW1_ICW4 0x01 //ICW4 will be present
#define ICW4_8086 0x01 //8086 mode

//write a byte to a hardware port
static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

//read a byte from a hardware port
static inline uint8_t inb(uint16_t port) {
    uint8_t value;
    __asm__ volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}    

//maps scancodes to ASCII characters, 0 means no printable character
static const char scancode_table[128] = {
    0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0,
    0, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', 0,
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    '-', 0, 0, 0, '+', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

//remap IRQ0-7 to vectors 0x20-0x27 and IRQ8-15 to 0x28-0x2F
//needed cause default BIOS mapping conflicts with CPU exceptions in protected mode
static void remap_pic(void) {
    //initialise both PICs
    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);

    //set new vector offsets
    outb(PIC1_DATA, 0x20);
    outb(PIC2_DATA, 0x28);

    //tell master PIC there is a slave at IRQ2, tell slave its cascade identity
    outb(PIC1_DATA, 0x04);
    outb(PIC2_DATA, 0x02);

    //set 8086 mode on both PICs
    outb(PIC1_DATA, ICW4_8086);
    outb(PIC2_DATA, ICW4_8086);

    //enable all interrupts
    outb(PIC1_DATA, 0x00);
    outb(PIC2_DATA, 0x00);
}

void init_irq(void) {
    remap_pic();
}

void irq_handler(uint32_t irq_number) {
    //call pit_tick on every timer interrupt to update the tick counter
    if (irq_number == 0) {
        pit_tick();
    }
    if (irq_number == 1) {
        uint8_t scancode = inb(0x60); //read scancode from keyboard port
        if (scancode < 128) { //ignore key release events (bit 7 set)
            char c = scancode_table[scancode];
            if (c != 0) {
                terminal_write_char(c);
            }
        }
    }
    //send EOI to slave PIC if IRQ came from it
    if (irq_number >= 8) {
        outb(PIC2_COMMAND, PIC_EOI);
    }
    //always send EOI to master PIC
    outb(PIC1_COMMAND, PIC_EOI);
}