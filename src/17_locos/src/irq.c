/*
Name: irq.c
Project: LocOS
Description: This file contains the implementation of the IRQ (Interrupt Request) handling for LocOS project
*/

#include "irq.h"
#include "pit.h"
#include "terminal.h"
#include <libc/stdbool.h>

#define PIC1_COMMAND 0x20
#define PIC1_DATA    0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA    0xA1

#define PIC_EOI      0x20
#define ICW1_INIT    0x10
#define ICW1_ICW4    0x01
#define ICW4_8086    0x01
#define KBD_DATA     0x60

#define KBD_BUF_SIZE 128

static uint8_t kbd_scancode_buffer[KBD_BUF_SIZE];
static uint32_t kbd_buf_head = 0;
/* Single-writer demo buffer: wraps when full (old bytes are overwritten). */

static uint8_t kbd_char_buffer[KBD_BUF_SIZE];
static uint32_t kbd_char_head = 0;
static uint32_t kbd_char_tail = 0;

static bool shift_down = false;
static bool caps_lock  = false;
static bool kbd_echo   = false;

/* PS/2 Set 1 scancode -> character maps (basic; minimal shift support). */
static const uint8_t scancode_unshifted[128] = {
    [0x01] = 27,  /* ESC */
    [0x02] = '1',  [0x03] = '2',  [0x04] = '3',  [0x05] = '4',
    [0x06] = '5',  [0x07] = '6',  [0x08] = '7',  [0x09] = '8',
    [0x0A] = '9',  [0x0B] = '0',  [0x0C] = '-',  [0x0D] = '=',
    [0x10] = 'q',  [0x11] = 'w',  [0x12] = 'e',  [0x13] = 'r',
    [0x14] = 't',  [0x15] = 'y',  [0x16] = 'u',  [0x17] = 'i',
    [0x18] = 'o',  [0x19] = 'p',
    [0x1A] = 0x86, /* å key */
    [0x1B] = ']',
    [0x1C] = '\n',
    [0x1E] = 'a',  [0x1F] = 's',  [0x20] = 'd',  [0x21] = 'f',
    [0x22] = 'g',  [0x23] = 'h',  [0x24] = 'j',  [0x25] = 'k',
    [0x26] = 'l',
    [0x27] = 0xF2, /* ; key -> ø (custom glyph slot) */
    [0x28] = 0x91, /* ' key -> æ */
    [0x29] = '`',
    [0x2B] = '\\',
    [0x2C] = 'z',  [0x2D] = 'x',  [0x2E] = 'c',  [0x2F] = 'v',
    [0x30] = 'b',  [0x31] = 'n',  [0x32] = 'm',  [0x33] = ',',
    [0x34] = '.',  [0x35] = '/',
    [0x39] = ' '
};

static const uint8_t scancode_shifted[128] = {
    [0x01] = 27,  /* ESC */
    [0x02] = '!',  [0x03] = '@',  [0x04] = '#',  [0x05] = '$',
    [0x06] = '%',  [0x07] = '^',  [0x08] = '&',  [0x09] = '*',
    [0x0A] = '(',  [0x0B] = ')',  [0x0C] = '_',  [0x0D] = '+',
    [0x10] = 'Q',  [0x11] = 'W',  [0x12] = 'E',  [0x13] = 'R',
    [0x14] = 'T',  [0x15] = 'Y',  [0x16] = 'U',  [0x17] = 'I',
    [0x18] = 'O',  [0x19] = 'P',
    [0x1A] = 0x8F, /* Å key */
    [0x1B] = '}',
    [0x1C] = '\n',
    [0x1E] = 'A',  [0x1F] = 'S',  [0x20] = 'D',  [0x21] = 'F',
    [0x22] = 'G',  [0x23] = 'H',  [0x24] = 'J',  [0x25] = 'K',
    [0x26] = 'L',
    [0x27] = 0xF3, /* : key -> Ø (custom glyph slot) */
    [0x28] = 0x92, /* " key -> Æ */
    [0x29] = '~',
    [0x2B] = '|',
    [0x2C] = 'Z',  [0x2D] = 'X',  [0x2E] = 'C',  [0x2F] = 'V',
    [0x30] = 'B',  [0x31] = 'N',  [0x32] = 'M',  [0x33] = '<',
    [0x34] = '>',  [0x35] = '?',
    [0x39] = ' '
};

static inline bool is_letter_code(uint8_t c) {
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
           c == 0x86 /* å */ || c == 0x8F /* Å */ ||
           c == 0x91 /* æ */ || c == 0x92 /* Æ */ ||
           c == 0xF2 /* ø */ || c == 0xF3 /* Ø */;
}

static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t value;
    __asm__ volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline void io_wait(void) {
    outb(0x80, 0);
}

void irq_init(void) {
    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();
    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();

    outb(PIC1_DATA, 0x20);  /* Master offset 0x20 (IRQ0..IRQ7) */
    io_wait();
    outb(PIC2_DATA, 0x28);  /* Slave  offset 0x28 (IRQ8..IRQ15) */
    io_wait();

    outb(PIC1_DATA, 4);     /* Tell Master about Slave at IRQ2 */
    io_wait();
    outb(PIC2_DATA, 2);     /* Tell Slave its cascade identity */
    io_wait();

    outb(PIC1_DATA, ICW4_8086);
    io_wait();
    outb(PIC2_DATA, ICW4_8086);
    io_wait();

    /* Keep it simple: unmask IRQ0 (PIT) and IRQ1 (keyboard). */
    outb(PIC1_DATA, 0xFC);
    outb(PIC2_DATA, 0xFF);
}

static void keyboard_handle_irq1(void) {
    uint8_t scancode = inb(KBD_DATA);
    /* Store raw scancode for debugging/report requirements. */
    kbd_scancode_buffer[kbd_buf_head % KBD_BUF_SIZE] = scancode;
    kbd_buf_head++;

    /* Track shift state (left/right). */
    if (scancode == 0x2A || scancode == 0x36) { /* press */
        shift_down = true;
        return;
    }
    if (scancode == 0xAA || scancode == 0xB6) { /* release */
        shift_down = false;
        return;
    }
    if (scancode == 0x3A) { /* Caps Lock toggle on press */
        caps_lock = !caps_lock;
        return;
    }

    /* Ignore key release events for this basic logger. */
    if (scancode & 0x80) {
        return;
    }

    uint8_t un = scancode_unshifted[scancode];
    uint8_t sh = scancode_shifted[scancode];

    bool letter = is_letter_code(un) || is_letter_code(sh);
    bool use_shift = letter ? (shift_down ^ caps_lock) : shift_down;

    char ch = (char)(use_shift ? sh : un);
    if (ch == '\0') {
        return;
    }

    /* Queue character for polling readers */
    uint32_t next_head = (kbd_char_head + 1) % KBD_BUF_SIZE;
    if (next_head != kbd_char_tail) { /* drop if full */
        kbd_char_buffer[kbd_char_head] = (uint8_t)ch;
        kbd_char_head = next_head;
    }

    if (kbd_echo) {
        char out[2] = { ch, '\0' };
        terminal_write(out);
    }
}

void irq_handler_c(uint32_t irq_no) {
    if (irq_no == 0) {
        pit_on_irq0();
    } else if (irq_no == 1) {
        keyboard_handle_irq1();
    } else {
        terminal_printf("IRQ %u triggered\n", (unsigned int)irq_no);
    }

    if (irq_no >= 8) {
        outb(PIC2_COMMAND, PIC_EOI);
    }
    outb(PIC1_COMMAND, PIC_EOI);
}

int kbd_try_getchar(void) {
    if (kbd_char_head == kbd_char_tail) {
        return -1;
    }
    uint8_t ch = kbd_char_buffer[kbd_char_tail];
    kbd_char_tail = (kbd_char_tail + 1) % KBD_BUF_SIZE;
    return (int)ch;
}

int kbd_getchar(void) {
    int c;
    while ((c = kbd_try_getchar()) == -1) {
        __asm__ volatile ("hlt");
    }
    return c;
}

void kbd_set_echo(bool enabled) {
    kbd_echo = enabled;
}
