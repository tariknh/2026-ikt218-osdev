#include "arch/i386/io.h"
#include "kernel/pit.h"

#include "stdio.h"

// Updated from the IRQ0 handler
static volatile uint32_t pit_ticks = 0;

void init_pit(void) {
    uint16_t divisor = PIT_DIVIDER;

    pit_ticks = 0;

    // Channel 0
    outb(PIT_CMD_PORT, 0x36);
    outb(PIT_CHANNEL0_PORT, (uint8_t) (divisor & 0xFF));
    outb(PIT_CHANNEL0_PORT, (uint8_t) (divisor >> 8 & 0xFF));
}

void pit_tick(void) {
    pit_ticks++;
}

uint32_t get_current_tick(void) {
    return pit_ticks;
}

void sleep_busy(uint32_t milliseconds) {
    uint32_t start_tick = get_current_tick();
    uint32_t ticks_to_wait = milliseconds * TICKS_PER_MS;
    uint32_t elapsed_ticks = 0;

    // Keep the CPU busy until the timer interrupt advances the global tick count
    while (elapsed_ticks < ticks_to_wait) {
        while (get_current_tick() == start_tick + elapsed_ticks) {
        }

        elapsed_ticks++;
    }
}

void sleep_interrupt(uint32_t milliseconds) {
    uint32_t current_tick = get_current_tick();
    uint32_t end_ticks = current_tick + milliseconds * TICKS_PER_MS;

    // Let IRQ0 wake the CPU on each PIT tick instead of burning cycles in a tight loop.
    while (current_tick < end_ticks) {
        __asm__ volatile ("sti; hlt");
        current_tick = get_current_tick();
    }
}
