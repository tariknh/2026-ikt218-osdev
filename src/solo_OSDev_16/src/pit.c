#include "pit.h"
#include "io.h"
#include <libc/stdint.h>

// Counts how many PIT timer ticks has occured
static volatile uint32_t pit_ticks = 0;

// Initialize PIT channel 0 to generate timer interrupts
void init_pit() {
    uint16_t divisor = (uint16_t)DIVIDER;

    // Tell the PIT to use channel 0, low/high byte access, mode 3, binary mode.
    outb(PIT_CMD_PORT, 0x36);       

    // Send the low byte of divisor to Channel 0
    outb(PIT_CHANNEL0_PORT, divisor & 0xFF);

    // Send the high byte of divisor to Channel 0
    outb(PIT_CHANNEL0_PORT, (divisor >> 8) & 0xFF);
}

// Returns current PIT tick count
uint32_t get_current_tick() {
    return pit_ticks;
}

// Called from IRQ0 each time the PIT sends a timer interrupt
void pit_tick(){
    pit_ticks++;
}

// Sleep by using interrupts instead of busy waiting
void sleep_interrupt(uint32_t milliseconds) {
    uint32_t current_tick = get_current_tick();
    uint32_t ticks_to_wait = milliseconds * TICKS_PER_MS;
    uint32_t end_ticks = current_tick + ticks_to_wait;

    // Enable interrupts and halt the CPU until the next interrupt occurs
    while(current_tick < end_ticks){
        __asm__ volatile("sti");
        __asm__ volatile("hlt");
        current_tick = get_current_tick();
    }
}

// Sleep by constantly checking the PIT tick counter
void sleep_busy(uint32_t milliseconds) {
    uint32_t start_tick = get_current_tick();
    uint32_t ticks_to_wait = milliseconds * TICKS_PER_MS;
    uint32_t elapsed_ticks = 0;

    while (elapsed_ticks < ticks_to_wait){
        while (get_current_tick() == start_tick + elapsed_ticks) {
            // Do nothing (busy wait)
        }
        elapsed_ticks++;
    }
}