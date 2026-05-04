#include <irq.h>
#include <isr.h>
#include <monitor.h>
#include <libc/stdint.h>

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ __volatile__("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static char scancode_ascii[128] = {
    0, 27, '1','2','3','4','5','6','7','8','9','0','-','=', 0, 0,
    'q','w','e','r','t','y','u','i','o','p','[',']', 0, 0,
    'a','s','d','f','g','h','j','k','l',';','\'','`', 0,'\\',
    'z','x','c','v','b','n','m',',','.','/', 0, 0, 0,' '
};

volatile char last_key = 0;

void keyboard_handler(registers_t* regs, void* context) {
    uint8_t scancode = inb(0x60);
    if (scancode < 128) {
        char c = scancode_ascii[scancode];
        if (c != 0) {
            last_key = c;
            monitor_put(c);
        }
    }
}

void init_keyboard(void) {
    register_irq_handler(IRQ1, keyboard_handler, 0);
}