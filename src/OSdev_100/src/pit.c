#include "../include/pit.h"
#include "../include/io.h" // Ensure you have outb defined here

static uint32_t tick = 0;

// this is basically our software clock, one tick is one millisecond here
uint32_t get_current_tick() {
    return tick;
}

// irq0 ends up here, so every PIT interrupt just increases tick
void timer_handler() {
    tick++;
}

// set up PIT channel 0 so it keeps sending timer interrupts
void init_pit() {
    outb(PIT_CMD_PORT, 0x36);

    uint16_t d = (uint16_t)DIVIDER;

    // PIT wants the divisor in two parts, low byte first and then high byte
    outb(PIT_CHANNEL0_PORT, (uint8_t)(d & 0xFF));
    outb(PIT_CHANNEL0_PORT, (uint8_t)((d >> 8) & 0xFF));
}

// simple but high CPU, we just keep spinning until enough ticks passed
void sleep_busy(uint32_t milliseconds) {
    uint32_t start_tick = get_current_tick();
    uint32_t ticks_to_wait = milliseconds * TICKS_PER_MS;
    uint32_t elapsed_ticks = 0;

    while (elapsed_ticks < ticks_to_wait) {
        while (get_current_tick() == start_tick + elapsed_ticks) {
            // do nothing, just wait for tick to change
        }
        elapsed_ticks++;
    }
}

// better sleep: let interrupts wake the CPU up instead of spinning the whole time
void sleep_interrupt(uint32_t milliseconds) {
    uint32_t current_tick = get_current_tick();
    uint32_t ticks_to_wait = milliseconds * TICKS_PER_MS;
    uint32_t end_ticks = current_tick + ticks_to_wait;

    while (get_current_tick() < end_ticks) {
        __asm__ volatile("sti; hlt");
    }
}
