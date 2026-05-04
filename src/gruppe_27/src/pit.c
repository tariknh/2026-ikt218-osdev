#include "pit.h"
#include "idt.h"
#include "terminal.h"

extern void outb(uint16_t port, uint8_t val);

static volatile uint32_t tick = 0;

volatile uint32_t get_current_tick() {
    return tick;
}

static void pit_handler(struct registers *r) {
    tick++;

}

void init_pit() {
    // Set PIT to mode 3 (square wave), channel 0, lo/hi byte access
    outb(PIT_CMD_PORT, 0x36);

    // Send the divisor low byte then high byte
    outb(PIT_CHANNEL0_PORT, (uint8_t)(DIVIDER & 0xFF));
    outb(PIT_CHANNEL0_PORT, (uint8_t)((DIVIDER >> 8) & 0xFF));

    // Register our handler for IRQ0 (the timer)
    irq_install_handler(0, pit_handler);

    terminal_write("[PIT] initialized at 1000 Hz\n");
}

void sleep_busy(uint32_t milliseconds) {
    uint32_t start_tick = get_current_tick();
    uint32_t ticks_to_wait = milliseconds * TICKS_PER_MS;

    while (get_current_tick() - start_tick < ticks_to_wait) {
    // Just wait for the total duration directly
    }
}

void sleep_interrupt(uint32_t milliseconds) {
    uint32_t current_tick = get_current_tick();
    uint32_t ticks_to_wait = milliseconds * TICKS_PER_MS;
    uint32_t end_ticks = current_tick + ticks_to_wait;

    while (current_tick < end_ticks) {
        __asm__ __volatile__ ("sti; hlt");
        current_tick = get_current_tick();
    }
}