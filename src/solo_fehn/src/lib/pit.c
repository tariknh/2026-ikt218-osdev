/*
 * pit.c - Programmable Interval Timer driver
 *
 * Configures the 8253/8254 PIT to fire IRQ0 every millisecond, and exposes
 * two sleep primitives:
 *
 *   sleep_busy(ms)        - polls get_current_tick() in a tight loop.
 *                            Easy to reason about, but pegs the CPU at 100%.
 *
 *   sleep_interrupt(ms)   - executes "sti; hlt" in a loop, putting the CPU
 *                            in halt-state until the next interrupt fires.
 *                            Wakes once per millisecond, checks the clock,
 *                            sleeps again if more time remains.
 */

#include <pit.h>
#include <irq.h>
#include <io.h>
#include <libc/stdint.h>

static volatile uint32_t tick_count = 0;

static void pit_callback(registers_t* r) {
    (void)r;
    tick_count++;
}

uint32_t get_current_tick(void) {
    return tick_count;
}

void init_pit(void) {
    /* Command byte 0x36:
       channel 0  (bits 6-7 = 00)
       lobyte/hibyte access mode (bits 4-5 = 11)
       mode 3 = square wave (bits 1-3 = 011)
       binary mode (bit 0 = 0) */
    outb(PIT_CMD_PORT, 0x36);

    /* Send the divisor low byte then high byte. */
    uint16_t divisor = (uint16_t)DIVIDER;
    outb(PIT_CHANNEL0_PORT, (uint8_t)(divisor & 0xFF));
    outb(PIT_CHANNEL0_PORT, (uint8_t)((divisor >> 8) & 0xFF));

    /* Register our IRQ0 handler and let the line through the PIC. */
    irq_register_handler(0, pit_callback);
    irq_unmask(0);
}

void sleep_busy(uint32_t milliseconds) {
    uint32_t start_tick    = get_current_tick();
    uint32_t ticks_to_wait = milliseconds * TICKS_PER_MS;
    uint32_t elapsed_ticks = 0;

    while (elapsed_ticks < ticks_to_wait) {
        /* Spin until the tick counter advances by one. */
        while (get_current_tick() == start_tick + elapsed_ticks) {
            /* busy wait */
        }
        elapsed_ticks++;
    }
}

void sleep_interrupt(uint32_t milliseconds) {
    uint32_t current_tick  = get_current_tick();
    uint32_t ticks_to_wait = milliseconds * TICKS_PER_MS;
    uint32_t end_ticks     = current_tick + ticks_to_wait;

    while (current_tick < end_ticks) {
        /* sti enables interrupts (so the next PIT IRQ can wake us);
           hlt halts the CPU until that interrupt fires. */
        __asm__ volatile ("sti");
        __asm__ volatile ("hlt");
        current_tick = get_current_tick();
    }
}
