#include "pit.h"

#define PIT_CMD_PORT      0x43
#define PIT_CHANNEL0_PORT 0x40

#define PIT_BASE_FREQUENCY 1193180
#define PIT_TARGET_HZ      1000
#define PIT_DIVIDER        (PIT_BASE_FREQUENCY / PIT_TARGET_HZ)

static volatile uint32_t pit_ticks = 0;

static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

uint32_t pit_get_ticks(void) {
    return pit_ticks;
}

void pit_on_irq0(void) {
    pit_ticks++;
}

void init_pit(void) {
    uint16_t divisor = (uint16_t)PIT_DIVIDER;

    /* Channel 0, lobyte/hibyte, mode 3 (square wave), binary counter. */
    outb(PIT_CMD_PORT, 0x36);
    outb(PIT_CHANNEL0_PORT, (uint8_t)(divisor & 0xFF));
    outb(PIT_CHANNEL0_PORT, (uint8_t)((divisor >> 8) & 0xFF));
}

void sleep_busy(uint32_t milliseconds) {
    uint32_t start = pit_get_ticks();
    uint32_t target = start + milliseconds;

    while (pit_get_ticks() < target) {
        __asm__ volatile ("pause");
    }
}

void sleep_interrupt(uint32_t milliseconds) {
    uint32_t start = pit_get_ticks();
    uint32_t target = start + milliseconds;

    while (pit_get_ticks() < target) {
        __asm__ volatile ("sti; hlt");
    }
}
