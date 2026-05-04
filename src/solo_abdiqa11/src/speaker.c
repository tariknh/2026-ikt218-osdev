#include "drivers/speaker.h"
#include "stdint.h"

/* I/O port helpers */
static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void enable_speaker(void) {
    uint8_t tmp = inb(0x61);
    if (tmp != (tmp | 3)) {
        outb(0x61, tmp | 3);
    }
}

void disable_speaker(void) {
    uint8_t tmp = inb(0x61) & 0xFC;
    outb(0x61, tmp);
}

void play_sound(uint32_t frequency) {
    if (frequency == 0) {
        return;
    }

    uint32_t divisor = 1193180 / frequency;

    outb(0x43, 0xB6);
    outb(0x42, (uint8_t)(divisor & 0xFF));
    outb(0x42, (uint8_t)((divisor >> 8) & 0xFF));

    enable_speaker();
}

void stop_sound(void) {
    uint8_t tmp = inb(0x61) & 0xFD;
    outb(0x61, tmp);
}
