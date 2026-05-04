#include "arch/i386/idt.h"
#include "arch/i386/io.h"
#include "arch/i386/keyboard.h"
#include "kernel/pit.h"
#include "stdbool.h"
#include "stdio.h"
#include "stdint.h"

#define PIC1_COMMAND 0x20
#define PIC1_DATA 0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA 0xA1
#define PIC_EOI 0x20
#define PIT_IRQ 32
#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_IRQ 33

/*
* Interrupt Service Routines (ISR)
*
* Reference:
* OSDev Wiki - Interrupt Service Routines
* https://wiki.osdev.org/Interrupt_Service_Routines
*/

extern void *isr_stub_table[];
extern void *irq_stub_table[];

// Keeps the raw scancode history that can be consumed later
static volatile char keyboard_buffer[KEY_SCANCODE_COUNT];
static volatile uint16_t keyboard_read_index = 0;
static volatile uint16_t keyboard_write_index = 0;

void keyboard_logger(void) {
    uint8_t scancode = inb(KEYBOARD_DATA_PORT);

    if ((scancode & KEY_SCANCODE_RELEASE_FLAG) != 0) {
        return;
    }

    char key = KEYBOARD_SCANCODE_TO_CHAR[scancode];
    if (key != 0) {
        uint16_t next_write_index = (uint16_t) ((keyboard_write_index + 1) % KEY_SCANCODE_COUNT);

        // Store both key press and key release scancodes
        keyboard_buffer[keyboard_write_index] = key;
        keyboard_write_index = next_write_index;
    }
}

bool keyboard_try_read(char *out) {
    if (keyboard_read_index == keyboard_write_index) {
        return false;
    }

    if (out != 0) {
        *out = keyboard_buffer[keyboard_read_index];
    }

    keyboard_read_index = (uint16_t) ((keyboard_read_index + 1) % KEY_SCANCODE_COUNT);
    return true;
}

char keyboard_wait_read(void) {
    char key = 0;

    // Halt while waiting for keyboard IRQ
    while (!keyboard_try_read(&key)) {
        __asm__ volatile ("sti; hlt");
    }

    return key;
}

void remap_pic(void) {
    outb(PIC1_COMMAND, 0x11);
    io_wait();
    outb(PIC2_COMMAND, 0x11);
    io_wait();

    outb(PIC1_DATA, 0x20);
    io_wait();
    outb(PIC2_DATA, 0x28);
    io_wait();

    outb(PIC1_DATA, 0x04);
    io_wait();
    outb(PIC2_DATA, 0x02);
    io_wait();

    outb(PIC1_DATA, 0x01);
    io_wait();
    outb(PIC2_DATA, 0x01);
    io_wait();

    outb(PIC1_DATA, 0x00);
    outb(PIC2_DATA, 0x00);
}

void init_isr(void) {
    remap_pic();

    for (uint8_t i = 0; i < 32; i++) {
        idt_set_gate(i, (uint32_t) isr_stub_table[i], 0x8E);
    }

    for (uint8_t i = 0; i < 16; i++) {
        idt_set_gate((uint8_t) (32 + i), (uint32_t) irq_stub_table[i], 0x8E);
    }
}

void isr_handler(uint32_t interrupt_number) {
    printf("Interrupt %i triggered\n", (int) interrupt_number);
}

void irq_handler(uint32_t interrupt_number) {
    // IRQ0 is the PIT and each timer interrupt advances the sleep tick counter
    if (interrupt_number == PIT_IRQ) {
        pit_tick();
    }

    if (interrupt_number == KEYBOARD_IRQ) {
        keyboard_logger();
    }

    if (interrupt_number >= 40) {
        outb(PIC2_COMMAND, PIC_EOI);
    }

    outb(PIC1_COMMAND, PIC_EOI);
}
