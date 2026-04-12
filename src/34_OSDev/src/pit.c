#include "pit.h"
#include <libc/stdio.h>

//tracks timer ticks since PIT was initialized
static volatile uint32_t tick_count = 0;

//called by the IRQ0 handler every time the timer triggers
void pit_tick() {
    tick_count++;
}

//returns current tick count
uint32_t get_tick_count() {
    return tick_count;
}

//write a byte to a hardware port
static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

//initialize PIT to tick at 1000 Hz (every millisecond)
void init_pit() {
    //tells PIT how often to interrupt the CPU
    outb(PIT_CMD_PORT, 0x36);

    //send divisor low byte first, then high byte
    outb(PIT_CHANNEL0_PORT, (uint8_t)(DIVIDER & 0xFF));
    outb(PIT_CHANNEL0_PORT, (uint8_t)((DIVIDER >> 8) & 0xFF));

    printf("PIT set to %d Hz\n", TARGET_FREQUENCY);
}

//loops until the requested number of ticks have passed (wasts CPU time)
void sleep_busy(uint32_t milliseconds) {
    uint32_t start_tick = tick_count;
    uint32_t ticks_to_wait = milliseconds * TICKS_PER_MS;
    uint32_t counted_ticks = 0;

    while (counted_ticks < ticks_to_wait) {
        while (tick_count == start_tick + counted_ticks) {}
        counted_ticks++;
    }
}

//pauses CPU between ticks (much more efficient for CPU)
void sleep_interrupt(uint32_t milliseconds) {
    uint32_t end_tick = tick_count + (milliseconds * TICKS_PER_MS);

    while (tick_count < end_tick) {
        __asm__ volatile ("sti; hlt");
    }
}