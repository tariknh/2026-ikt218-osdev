#include "pit.h"

// global tick counter (THIS IS YOUR KERNEL CLOCK)
static volatile uint32_t pit_ticks = 0;

// =====================
// REQUIRED FUNCTIONS
// =====================

uint32_t get_current_tick() {
    return pit_ticks;
}

// IRQ0 handler (called by your IDT entry 32)
void pit_irq_handler() {
    pit_ticks++;

    // End of interrupt signal to PIC
    outb(PIC1_CMD_PORT, PIC_EOI);
}

// =====================
// PIT INITIALIZATION
// =====================

void init_pit() {
    uint16_t divisor = DIVIDER;

    // Channel 0, lobyte/hibyte, mode 3 (square wave), binary
    outb(PIT_CMD_PORT, 0x36);

    outb(PIT_CHANNEL0_PORT, (uint8_t)(divisor & 0xFF));
    outb(PIT_CHANNEL0_PORT, (uint8_t)((divisor >> 8) & 0xFF));
}

// =====================
// SLEEP (INTERRUPT)
// =====================

void sleep_interrupt(uint32_t milliseconds) {
    uint32_t start = get_current_tick();
    uint32_t ticks_to_wait = milliseconds * TICKS_PER_MS;
    uint32_t end = start + ticks_to_wait;

    while (get_current_tick() < end) {
        asm volatile("sti");
        asm volatile("hlt");
    }
}

// =====================
// SLEEP (BUSY WAIT)
// =====================

void sleep_busy(uint32_t milliseconds) {
    uint32_t start = get_current_tick();
    uint32_t ticks_to_wait = milliseconds * TICKS_PER_MS;

    for (uint32_t i = 0; i < ticks_to_wait; i++) {
        while (get_current_tick() == start + i) {
            // busy wait
        }
    }
}