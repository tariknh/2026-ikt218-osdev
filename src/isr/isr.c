#include "isr.h"
#include "libc/stdbool.h"
#include "libc/stddef.h"
#include "libc/stdint.h"

// Vi setter opp bare de viktigste for nå:
// 0: Divide by Zero
// 1: Debug Exception  
// 14: Page Fault
// Alle må håndteres før interrupts er enablet, ellers får vi tripple fault + reboot.

enum {
    VGA_WIDTH = 80,
    VGA_HEIGHT = 25,
    VGA_COLOR = 0x0F,
    KERNEL_CODE_SELECTOR = 0x08,
    INTERRUPT_GATE_FLAGS = 0x8E
};

typedef struct {
    uint16_t offset_low;      // Laveste 16 bits av handler-addressa
    uint16_t selector;        // Code segment selector (GDT index)
    uint8_t zero;             // Reservert - må være 0
    uint8_t type_attributes;  // Gate type og permission bits
    uint16_t offset_high;     // Høyeste 16 bits av handler-addressa
} __attribute__((packed)) idt_entry_t;

//IDTR-register
typedef struct {
    uint16_t limit;  // Size of IDT - 1
    uint32_t base;   // Base address av IDT
} __attribute__((packed)) idtr_t;

static idt_entry_t idt[256];
static idtr_t idtr;

// VGA console for exception output
static volatile uint16_t* const vga_buffer = (volatile uint16_t*)0xB8000;
static size_t cursor_row = 0;
static size_t cursor_column = 0;
static bool console_ready = false;

void idt_set_gate(uint8_t vector, void (*handler)(void)) {
    uint32_t handler_address = (uint32_t)handler;

    idt[vector].offset_low = (uint16_t)(handler_address & 0xFFFF);
    idt[vector].selector = KERNEL_CODE_SELECTOR;
    
    // Reservert byte - må være 0
    idt[vector].zero = 0;
    
    // Gate type: 0x8E = interrupt gate, 32-bit, kernel privilege level
    idt[vector].type_attributes = INTERRUPT_GATE_FLAGS;
    idt[vector].offset_high = (uint16_t)((handler_address >> 16) & 0xFFFF);
}

static void idt_load(const idtr_t* descriptor) {
    // lidt=%0 = load IDT with descriptor at memory location
    asm volatile("lidt %0" : : "m"(*descriptor));
}

void isr_initialize(void) {
    // Null ut hele IDT. vi bruker bare 0, 1 og 14 for nå
    for (size_t index = 0; index < 256; ++index) {
        idt[index].offset_low = 0;
        idt[index].selector = 0;
        idt[index].zero = 0;
        idt[index].type_attributes = 0;
        idt[index].offset_high = 0;
    }

    idt_set_gate(0, isr0); //divided by zero
    idt_set_gate(1, isr1); //debug exception
    idt_set_gate(14, isr14); //page fault

    // Sett opp IDTR register med IDT-info
    idtr.limit = (uint16_t)(sizeof(idt) - 1);  // Size = 256*8 - 1
    idtr.base = (uint32_t)&idt[0];             // Base address av IDT array
    
    // Last IDT i CPU
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
    // Guard: run bare en gang
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

// Skriv en null-terminert string
static void console_write(const char* message) {
    for (size_t index = 0; message[index] != '\0'; ++index) {
        console_putc(message[index]);
    }
}

// CPU Exception Handlers
// Disse kalles når CPU får en exception (ikke IRQ).
// Vi initialiserer konsollen første gang så vi kan skrive error-meldinger.

void isr0_handler(void) {
    console_initialize();
    console_write("ISR 0 triggered: divide-by-zero\n");
    
    // Hang forever (vi kan ikke recovery fra dette i kernel-kontekst)
    while (1) {
        asm volatile("hlt");  // Halt CPU
    }
}

void isr1_handler(void) {
    console_initialize();
    console_write("ISR 1 triggered: debug interrupt\n");
    
    // Hang (debug exceptions skal ikke skje i normal operasjon)
    while (1) {
        asm volatile("hlt");
    }
}

void isr14_handler(void) {
    console_initialize();
    console_write("ISR 14 triggered: page fault interrupt\n");
    
    // Hang (page fault = memory corruption, ikke recoverable)
    while (1) {
        asm volatile("hlt");
    }
}
