#include <pit.h>
#include <pic.h>        /* outb, io_wait */
#include <libc/stdint.h>

/* Global tick counter – incremented by pit_handler() on every IRQ0 */
static volatile uint32_t pit_ticks = 0;

/* Return the current tick count */
uint32_t get_current_tick(void)
{
    return pit_ticks;
}

/* Called from irq_handler() whenever IRQ0 fires */
void pit_handler(void)
{
    pit_ticks++;
}

/* Initialise the PIT to fire at TARGET_FREQUENCY (1000 Hz = every 1 ms) */
void init_pit(void)
{
    uint16_t divisor = DIVIDER;

    /* Channel 0, lo/hi byte, rate generator (mode 2) */
    outb(PIT_CMD_PORT, 0x34);

    /* Send divisor low byte then high byte */
    outb(PIT_CHANNEL0_PORT, (uint8_t)(divisor & 0xFF));
    outb(PIT_CHANNEL0_PORT, (uint8_t)((divisor >> 8) & 0xFF));
}

/* Sleep using busy-waiting */
void sleep_busy(uint32_t milliseconds)
{
    uint32_t start_tick = get_current_tick();
    uint32_t ticks_to_wait = milliseconds * TICKS_PER_MS;
    uint32_t elapsed_ticks = 0;

    while (elapsed_ticks < ticks_to_wait)
    {
        /* Spin until the tick counter advances */
        while (get_current_tick() == start_tick + elapsed_ticks)
        {
            /* busy wait */
        }
        elapsed_ticks++;
    }
}

/* Sleep using interrupts (CPU halts between ticks – low power) */
void sleep_interrupt(uint32_t milliseconds)
{
    uint32_t current_tick = get_current_tick();
    uint32_t ticks_to_wait = milliseconds * TICKS_PER_MS;
    uint32_t end_ticks = current_tick + ticks_to_wait;

    while (current_tick < end_ticks)
    {
        __asm__ volatile("sti");
        __asm__ volatile("hlt");
        current_tick = get_current_tick();
    }
}
