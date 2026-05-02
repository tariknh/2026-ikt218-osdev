#include "pit.h"
#include "io.h"
#include "terminal.h"
// This file contains simple implementation of the PIT, for basic sleeping functionality. 

#define PIT_FREQUENCY 1000 

static volatile uint32_t tick_count = 0;
// PIT handler, called upon inside the IRQ0 handler. 
void pit_handler() {
    tick_count++; // Increment the tickcount on the PIT
}

uint32_t get_tick_count(void) {
    return tick_count;
}

void pit_init() {
    uint16_t divisor = 1193180 / PIT_FREQUENCY; // Defining the divisor for the desired frequency

    outb(0x43, 0x36);  // Channel: 0
    outb(0x40, (uint8_t)(divisor & 0xFF));  // Channel 0 data port, low byte
    outb(0x40, (uint8_t)((divisor >> 8) & 0xFF));  // Channel 0 data port, high byte
    printf("PIT initialized at 1kHz\n");
}

// ====== Sleep functions =======

void sleep_interrupt(uint32_t milliseconds) { // Sleep function using interrupts 
    uint32_t start_ticks = tick_count;
    while ((tick_count - start_ticks) < milliseconds) {
        __asm__ __volatile__("hlt"); // Halt the CPU until the next interrupt (PIT will wake it up)
    }
}

void sleep_busy(uint32_t milliseconds) { // Sleep function using busy-waiting 
    uint32_t start_ticks = tick_count;
    while ((tick_count - start_ticks) < milliseconds) {
        __asm__ __volatile__("pause"); // pause instruction to reduce CPU power consumption during busy-waiting
    }
}