#include "pit.h"
#include "ports.h"
#include "irq.h"
#include "libc/stdint.h"

// PIT I/O ports
#define PIT_COMMAND_PORT   0x43
#define PIT_CHANNEL0_PORT  0x40

static volatile uint32_t pit_ticks = 0; // Number of PIT ticks since startup
static uint32_t pit_frequency = 100; // 100 Hz = 10 ms per tick

// Called every timer interrupt
static void pit_callback(struct registers* regs) {
    (void)regs;
    pit_ticks++;
}

// Initializes the PIT timer
void init_pit(void) {
    uint32_t divisor = PIT_BASE_FREQUENCY / pit_frequency;     // Calculate PIT divisor

    outb(PIT_COMMAND_PORT, 0x36);     // Configure PIT channel 0
    outb(PIT_CHANNEL0_PORT, (uint8_t)(divisor & 0xFF));     // Send low byte of divisor
    outb(PIT_CHANNEL0_PORT, (uint8_t)((divisor >> 8) & 0xFF));    // Send high byte of divisor

    irq_install_handler(0, pit_callback);     // Install timer IRQ handler
}

// Returns current tick count
uint32_t pit_get_ticks(void) {
    return pit_ticks;
}

// Sleeps for a number of milliseconds
void pit_sleep_ms(uint32_t ms) {
    uint32_t ticks_to_wait = (ms * pit_frequency) / 1000;     // Convert milliseconds to ticks

    // Wait at least one tick
    if (ticks_to_wait == 0) {
        ticks_to_wait = 1;
    }

    uint32_t start = pit_get_ticks();     // Store start tick
    // Wait until enough ticks have passed
    while ((pit_get_ticks() - start) < ticks_to_wait) {
        __asm__ volatile ("hlt");
    }
}