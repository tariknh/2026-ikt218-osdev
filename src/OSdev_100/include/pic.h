#pragma once

#include "libc/stdint.h"

// PIC port addresses
// Master PIC handles IRQs 0-7, Slave PIC handles IRQs 8-15
#define PIC1_COMMAND 0x20  // master PIC command port
#define PIC1_DATA    0x21  // master PIC data port
#define PIC2_COMMAND 0xA0  // slave PIC command port
#define PIC2_DATA    0xA1  // slave PIC data port


// PIC commands
#define PIC_EOI      0x20  // end of interrupt command



// Remap IRQs to these interrupt vectors
// IRQs 0-7  → interrupts 32-39
// IRQs 8-15 → interrupts 40-47
#define PIC1_OFFSET  0x20  // 32 in decimal
#define PIC2_OFFSET  0x28  // 40 in decimal

// sends a byte to a hardware port
void outb(uint16_t port, uint8_t value);

// reads a byte from a hardware port  
uint8_t inb(uint16_t port);

// small delay for PIC to process commands
void io_wait(void);

// remaps PIC so IRQs don't conflict with CPU exceptions
void pic_init(void);

// tells PIC we finished handling an interrupt
void pic_send_eoi(uint8_t irq);
