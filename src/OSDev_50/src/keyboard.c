#include <keyboard.h>
#include <interrupts.h>
#include <common.h>
#include <input.h>
#include <terminal.h>

#define KBD_BUF_SIZE 128
static volatile uint32_t g_kbd_irq_count = 0;

uint32_t keyboard_irq_count(void) {
    return g_kbd_irq_count;
}

static volatile uint8_t g_last_scancode = 0;

uint8_t keyboard_last_scancode(void) {
    uint8_t s = g_last_scancode;
    g_last_scancode = 0;
    return s;
}
static volatile char g_last_key = 0;

char keyboard_last_key(void) {
    char c = g_last_key;
    g_last_key = 0;
    return c;
}

static uint8_t buf[KBD_BUF_SIZE];
static uint32_t head = 0;

static void push_scancode(uint8_t sc)
{
    buf[head % KBD_BUF_SIZE] = sc;
    head++;
}

static void keyboard_irq1(registers_t* regs, void* ctx)
{
    (void)regs; (void)ctx;
    g_kbd_irq_count++;

    uint8_t sc = inb(0x60);
    push_scancode(sc);
    
    if ((sc & 0x80) == 0) {        
        g_last_scancode = (uint8_t)sc;

    // DEBUG: vis at IRQ1 faktisk trigges (top-right corner)
        volatile uint16_t* vga = (volatile uint16_t*)0xB8000;
        vga[0 * 80 + 78] = (uint16_t)('K' | (0x4F << 8));  
    }

    char c = scancode_to_ascii(sc);
    if (c) {
        g_last_key = c;
        char s[2];
        s[0] = c;
        s[1] = 0;
        //terminal_write(s);

    }
}

void keyboard_init(void)
{
    register_irq_handler(1, keyboard_irq1, 0);
}
