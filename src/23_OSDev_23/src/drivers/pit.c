#include <stdint.h>
#include "ports.h"
#include "irq.h"
#include "pit.h"

// Global tick counter incremented by the IRQ0 handler
static volatile uint32_t pit_ticks = 0;

// IRQ0 handler — called on every PIT interrupt
static void pit_handler(registers_t* r) {
    (void)r;
    pit_ticks++;
}

// Returns the current tick count
static uint32_t get_current_tick() {
    return pit_ticks;
}

void init_pit() {
    // Register IRQ0 handler
    irq_install_handler(0, pit_handler);

    // Program PIT channel 0 in mode 3 (square wave), binary counting
    // Command: channel 0, lobyte/hibyte, mode 3, binary
    outb(PIT_CMD_PORT, 0x36);

    // Set divisor (lo byte then hi byte)
    uint16_t divisor = DIVIDER;
    outb(PIT_CHANNEL0_PORT, (uint8_t)(divisor & 0xFF));
    outb(PIT_CHANNEL0_PORT, (uint8_t)((divisor >> 8) & 0xFF));
}

// Sleep using interrupts (LOW CPU): enables interrupts and halts until done
void sleep_interrupt(uint32_t milliseconds) {
    uint32_t start_tick = get_current_tick();
    uint32_t ticks_to_wait = milliseconds * TICKS_PER_MS;
    uint32_t end_ticks = start_tick + ticks_to_wait;

    while (get_current_tick() < end_ticks) {
        // Enable interrupts and halt until the next interrupt fires
        asm volatile("sti");
        asm volatile("hlt");
        // Set current tick to get fresh value each iteration
    }
}

// Sleep using busy-waiting (HIGH CPU): spin-loop until done
void sleep_busy(uint32_t milliseconds) {
    uint32_t start_tick = get_current_tick();
    uint32_t ticks_to_wait = milliseconds * TICKS_PER_MS;
    uint32_t elapsed_ticks = 0;

    while (elapsed_ticks < ticks_to_wait) {
        // a. Do nothing (busy wait)
        // b. Increment elapsed ticks
        elapsed_ticks = get_current_tick() - start_tick;
    }
}
