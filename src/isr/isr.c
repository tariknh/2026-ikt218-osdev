#include "isr.h"
#include "libc/stdbool.h"
#include "libc/stddef.h"
#include "libc/stdint.h"

// Vi håndterer foreløpig bare de vanligste kritiske CPU-exceptionene.

enum {
    VGA_WIDTH = 80,
    VGA_HEIGHT = 25,
    VGA_COLOR = 0x0F,
    KERNEL_CODE_SELECTOR = 0x08,
    INTERRUPT_GATE_FLAGS = 0x8E
};

typedef struct {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t zero;
    uint8_t type_attributes;
    uint16_t offset_high;
} __attribute__((packed)) idt_entry_t;

typedef struct {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) idtr_t;

static idt_entry_t idt[256];
static idtr_t idtr;

static volatile uint16_t* const vga_buffer = (volatile uint16_t*)0xB8000;
static size_t cursor_row = 0;
static size_t cursor_column = 0;
static bool console_ready = false;

void idt_set_gate(uint8_t vector, void (*handler)(void)) {
    uint32_t handler_address = (uint32_t)handler;

    idt[vector].offset_low = (uint16_t)(handler_address & 0xFFFF);
    idt[vector].selector = KERNEL_CODE_SELECTOR;
    
    idt[vector].zero = 0;

    idt[vector].type_attributes = INTERRUPT_GATE_FLAGS;
    idt[vector].offset_high = (uint16_t)((handler_address >> 16) & 0xFFFF);
}

static void idt_load(const idtr_t* descriptor) {
    asm volatile("lidt %0" : : "m"(*descriptor));
}

void isr_initialize(void) {
    for (size_t index = 0; index < 256; ++index) {
        idt[index].offset_low = 0;
        idt[index].selector = 0;
        idt[index].zero = 0;
        idt[index].type_attributes = 0;
        idt[index].offset_high = 0;
    }

    idt_set_gate(0, isr0);
    idt_set_gate(1, isr1);
    idt_set_gate(14, isr14);

    idtr.limit = (uint16_t)(sizeof(idt) - 1);
    idtr.base = (uint32_t)&idt[0];

    idt_load(&idtr);
}

static void console_clear(void) {
    for (size_t row = 0; row < VGA_HEIGHT; ++row) {
        for (size_t column = 0; column < VGA_WIDTH; ++column) {
            vga_buffer[row * VGA_WIDTH + column] = (uint16_t)(VGA_COLOR << 8) | ' ';
        }
    }
}

static void console_advance_line(void) {
    cursor_column = 0;
    ++cursor_row;

    if (cursor_row >= VGA_HEIGHT) {
        console_clear();
        cursor_row = 0;
    }
}

static void console_initialize(void) {
    if (console_ready) {
        return;
    }

    console_clear();
    cursor_row = 0;
    cursor_column = 0;
    console_ready = true;
}

static void console_putc(char character) { 
    if (character == '\n') {
        console_advance_line();
        return;
    }

    vga_buffer[cursor_row * VGA_WIDTH + cursor_column] =
        (uint16_t)(VGA_COLOR << 8) | (uint8_t)character;

    ++cursor_column;
    if (cursor_column >= VGA_WIDTH) {
        console_advance_line();
    }
}

static void console_write(const char* message) {
    for (size_t index = 0; message[index] != '\0'; ++index) {
        console_putc(message[index]);
    }
}

// Vi stopper CPU-en etter en alvorlig exception for å unngå videre skade.

void isr0_handler(void) {
    console_initialize();
    console_write("ISR 0 triggered: divide-by-zero\n");

    while (1) {
        asm volatile("hlt");
    }
}

void isr1_handler(void) {
    console_initialize();
    console_write("ISR 1 triggered: debug interrupt\n");

    while (1) {
        asm volatile("hlt");
    }
}

void isr14_handler(void) {
    console_initialize();
    console_write("ISR 14 triggered: page fault interrupt\n");

    while (1) {
        asm volatile("hlt");
    }
}
