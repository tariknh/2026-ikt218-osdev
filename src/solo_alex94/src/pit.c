#include "kernel/pit.h"
#include "irq.h"
#include <libc/stdint.h>

static volatile uint32_t timer_ticks = 0;

static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static void pit_irq_callback(struct registers* regs) {
    (void)regs;
    timer_ticks++;
}

uint32_t pit_get_ticks(void) {
    return timer_ticks;
}

void init_pit(void) {
    uint16_t divisor = (uint16_t)DIVIDER;

    register_irq_handler(0, pit_irq_callback);

    outb(PIT_CMD_PORT, 0x36);
    outb(PIT_CHANNEL0_PORT, (uint8_t)(divisor & 0xFF));
    outb(PIT_CHANNEL0_PORT, (uint8_t)((divisor >> 8) & 0xFF));
}

void sleep_busy(uint32_t milliseconds) {
    uint32_t start_tick = pit_get_ticks();
    uint32_t ticks_to_wait = milliseconds * TICKS_PER_MS;

    while ((pit_get_ticks() - start_tick) < ticks_to_wait) {
        /* busy wait */
    }
}

void sleep_interrupt(uint32_t milliseconds) {
    uint32_t start_tick = pit_get_ticks();
    uint32_t ticks_to_wait = milliseconds * TICKS_PER_MS;

    while ((pit_get_ticks() - start_tick) < ticks_to_wait) {
        __asm__ volatile("sti; hlt");
    }
}
