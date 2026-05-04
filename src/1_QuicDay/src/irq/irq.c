#include "irq.h"
#include "isr.h"
#include "pit.h"
#include "libc/stdbool.h"
#include "libc/stddef.h"
#include "libc/stdint.h"

#define PIC_MASTER_CMD   0x20
#define PIC_MASTER_DATA  0x21
#define PIC_SLAVE_CMD    0xA0
#define PIC_SLAVE_DATA   0xA1
#define KEYBOARD_DATA    0x60

// Kommandoverdier for PIC-oppsett og tastaturstatus.
#define ICW1             0x11
#define ICW4             0x01
#define EOI              0x20
#define KEY_RELEASE_MASK 0x80

#define LEFT_SHIFT_PRESS   0x2A
#define RIGHT_SHIFT_PRESS  0x36
#define LEFT_SHIFT_RELEASE 0xAA
#define RIGHT_SHIFT_RELEASE 0xB6

#define IRQ_BASE         32
#define IRQ_SLAVE_BASE   40

#define KEYBOARD_SCANCODE_BUFFER_SIZE 256

enum {
    VGA_WIDTH = 80,
    VGA_HEIGHT = 25,
    VGA_COLOR = 0x0F
};

static volatile uint16_t* const vga_buffer = (volatile uint16_t*)0xB8000;
static size_t cursor_row = 0;
static size_t cursor_column = 0;

static uint8_t keyboard_scancode_buffer[KEYBOARD_SCANCODE_BUFFER_SIZE];
static size_t keyboard_scancode_head = 0;
static size_t keyboard_scancode_tail = 0;
static bool keyboard_shift_active = false;
static volatile uint32_t keyboard_event_count = 0;


// tast set 1 oversettes til ASCII her.
static const char scancode_set1_ascii[128] = {
    [0x01] = 27,
    [0x02] = '1', [0x03] = '2', [0x04] = '3', [0x05] = '4',
    [0x06] = '5', [0x07] = '6', [0x08] = '7', [0x09] = '8',
    [0x0A] = '9', [0x0B] = '0', [0x0C] = '-', [0x0D] = '=',
    [0x0E] = '\b', [0x0F] = '\t',
    [0x10] = 'q', [0x11] = 'w', [0x12] = 'e', [0x13] = 'r',
    [0x14] = 't', [0x15] = 'y', [0x16] = 'u', [0x17] = 'i',
    [0x18] = 'o', [0x19] = 'p', [0x1A] = '[', [0x1B] = ']',
    [0x1C] = '\n',
    [0x1E] = 'a', [0x1F] = 's', [0x20] = 'd', [0x21] = 'f',
    [0x22] = 'g', [0x23] = 'h', [0x24] = 'j', [0x25] = 'k',
    [0x26] = 'l', [0x27] = ';', [0x28] = '\'', [0x29] = '`',
    [0x2B] = '\\',
    [0x2C] = 'z', [0x2D] = 'x', [0x2E] = 'c', [0x2F] = 'v',
    [0x30] = 'b', [0x31] = 'n', [0x32] = 'm', [0x33] = ',',
    [0x34] = '.', [0x35] = '/',
    [0x39] = ' '
};

static const char scancode_set1_ascii_shift[128] = {
    [0x01] = 27,
    [0x02] = '!', [0x03] = '@', [0x04] = '#', [0x05] = '$',
    [0x06] = '%', [0x07] = '^', [0x08] = '&', [0x09] = '*',
    [0x0A] = '(', [0x0B] = ')', [0x0C] = '_', [0x0D] = '+',
    [0x0E] = '\b', [0x0F] = '\t',
    [0x10] = 'Q', [0x11] = 'W', [0x12] = 'E', [0x13] = 'R',
    [0x14] = 'T', [0x15] = 'Y', [0x16] = 'U', [0x17] = 'I',
    [0x18] = 'O', [0x19] = 'P', [0x1A] = '{', [0x1B] = '}',
    [0x1C] = '\n',
    [0x1E] = 'A', [0x1F] = 'S', [0x20] = 'D', [0x21] = 'F',
    [0x22] = 'G', [0x23] = 'H', [0x24] = 'J', [0x25] = 'K',
    [0x26] = 'L', [0x27] = ':', [0x28] = '"', [0x29] = '~',
    [0x2B] = '|',
    [0x2C] = 'Z', [0x2D] = 'X', [0x2E] = 'C', [0x2F] = 'V',
    [0x30] = 'B', [0x31] = 'N', [0x32] = 'M', [0x33] = '<',
    [0x34] = '>', [0x35] = '?',
    [0x39] = ' '
};

static void keyboard_buffer_push(uint8_t scancode) {
    keyboard_scancode_buffer[keyboard_scancode_head] = scancode;
    keyboard_scancode_head = (keyboard_scancode_head + 1) % KEYBOARD_SCANCODE_BUFFER_SIZE;

    // Hvis bufferen er full, dropp den eldste tegn.
    if (keyboard_scancode_head == keyboard_scancode_tail) {
        keyboard_scancode_tail = (keyboard_scancode_tail + 1) % KEYBOARD_SCANCODE_BUFFER_SIZE;
    }
}

static void keyboard_console_advance_line(void) {
    cursor_column = 0;
    ++cursor_row;

    if (cursor_row >= VGA_HEIGHT) {
        cursor_row = 0;
    }
}

static void keyboard_console_putc(char character) {
    if (character == '\n') {
        keyboard_console_advance_line();
        return;
    }

    // Enkel tab-støtte: fire mellomrom.
    if (character == '\t') {
        for (size_t index = 0; index < 4; ++index) {
            keyboard_console_putc(' ');
        }
        return;
    }

    if (character == '\b') {
        if (cursor_column > 0) {
            --cursor_column;
        } else if (cursor_row > 0) {
            --cursor_row;
            cursor_column = VGA_WIDTH - 1;
        }
        vga_buffer[cursor_row * VGA_WIDTH + cursor_column] =
            (uint16_t)(VGA_COLOR << 8) | ' ';
        return;
    }

    vga_buffer[cursor_row * VGA_WIDTH + cursor_column] =
        (uint16_t)(VGA_COLOR << 8) | (uint8_t)character;

    ++cursor_column;
    if (cursor_column >= VGA_WIDTH) {
        keyboard_console_advance_line();
    }
}

static char keyboard_translate_scancode(uint8_t scancode) {
    if (scancode >= 128) {
        return 0;
    }

    if (keyboard_shift_active) {
        return scancode_set1_ascii_shift[scancode];
    }

    return scancode_set1_ascii[scancode];
}

// Lavnivå I/O mot porter.
static inline void outb(uint16_t port, uint8_t value) {
    asm volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t value;
    asm volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

// Flytter IRQ-vektorer vekk fra CPU exceptions (0-31).
static void pic_remap(void) {
    uint8_t master_mask = inb(PIC_MASTER_DATA);
    uint8_t slave_mask = inb(PIC_SLAVE_DATA);

    outb(PIC_MASTER_CMD, ICW1);
    outb(PIC_SLAVE_CMD, ICW1);

    outb(PIC_MASTER_DATA, IRQ_BASE);
    outb(PIC_SLAVE_DATA, IRQ_SLAVE_BASE);

    outb(PIC_MASTER_DATA, 0x04);
    outb(PIC_SLAVE_DATA, 0x02);

    outb(PIC_MASTER_DATA, ICW4);
    outb(PIC_SLAVE_DATA, ICW4);

    outb(PIC_MASTER_DATA, master_mask);
    outb(PIC_SLAVE_DATA, slave_mask);
}

static void pic_send_eoi(uint8_t irq) {
    if (irq >= 8) {
        outb(PIC_SLAVE_CMD, EOI);
    }
    // Master skal alltid ha EOI, også når avbruddet kom fra slave.
    outb(PIC_MASTER_CMD, EOI);
}

void irq_initialize(void) {
    pic_remap();

    idt_set_gate(IRQ_BASE + 0, irq0);
    idt_set_gate(IRQ_BASE + 1, irq1);
    idt_set_gate(IRQ_BASE + 2, irq2);
    idt_set_gate(IRQ_BASE + 3, irq3);
    idt_set_gate(IRQ_BASE + 4, irq4);
    idt_set_gate(IRQ_BASE + 5, irq5);
    idt_set_gate(IRQ_BASE + 6, irq6);
    idt_set_gate(IRQ_BASE + 7, irq7);
    idt_set_gate(IRQ_BASE + 8, irq8);
    idt_set_gate(IRQ_BASE + 9, irq9);
    idt_set_gate(IRQ_BASE + 10, irq10);
    idt_set_gate(IRQ_BASE + 11, irq11);
    idt_set_gate(IRQ_BASE + 12, irq12);
    idt_set_gate(IRQ_BASE + 13, irq13);
    idt_set_gate(IRQ_BASE + 14, irq14);
    idt_set_gate(IRQ_BASE + 15, irq15);

    outb(PIC_MASTER_DATA, 0x00);
    outb(PIC_SLAVE_DATA, 0x00);
}

void irq0_handler(void) {
    pit_on_tick();
    pic_send_eoi(0);
}

void irq1_handler(void) {
    uint8_t scancode = inb(KEYBOARD_DATA);
    ++keyboard_event_count;
    keyboard_buffer_push(scancode);

    if (scancode == LEFT_SHIFT_PRESS || scancode == RIGHT_SHIFT_PRESS) {
        keyboard_shift_active = true;
        pic_send_eoi(1);
        return;
    }

    if (scancode == LEFT_SHIFT_RELEASE || scancode == RIGHT_SHIFT_RELEASE) {
        keyboard_shift_active = false;
        pic_send_eoi(1);
        return;
    }

    // Vi skriver bare ved key press, ikke key release.
    if ((scancode & KEY_RELEASE_MASK) == 0) {
        char character = keyboard_translate_scancode(scancode);
        if (character != 0) {
            keyboard_console_putc(character);
        }
    }

    pic_send_eoi(1);
}

uint32_t irq_get_keyboard_event_count(void) {
    return keyboard_event_count;
}

void irq2_handler(void) {
    pic_send_eoi(2);
}

void irq3_handler(void) {
    pic_send_eoi(3);
}

void irq4_handler(void) {
    pic_send_eoi(4);
}

void irq5_handler(void) {
    pic_send_eoi(5);
}

void irq6_handler(void) {
    pic_send_eoi(6);
}

void irq7_handler(void) {
    pic_send_eoi(7);
}

void irq8_handler(void) {
    pic_send_eoi(8);
}

void irq9_handler(void) {
    pic_send_eoi(9);
}

void irq10_handler(void) {
    pic_send_eoi(10);
}

void irq11_handler(void) {
    pic_send_eoi(11);
}

void irq12_handler(void) {
    pic_send_eoi(12);
}

void irq13_handler(void) {
    pic_send_eoi(13);
}

void irq14_handler(void) {
    pic_send_eoi(14);
}

void irq15_handler(void) {
    pic_send_eoi(15);
}
