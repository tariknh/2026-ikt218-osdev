#include "kernel/pit.h"
#include "io.h"
#include "libc/stdio.h"

static volatile uint32_t pit_ticks = 0;

void pit_tick(void)
{
    pit_ticks++;
}

uint32_t get_current_tick(void)
{
    return pit_ticks;
}

void init_pit(void)
{
    uint32_t divisor = DIVIDER;

    outb(PIT_CMD_PORT, 0x36);
    outb(PIT_CHANNEL0_PORT, (uint8_t)(divisor & 0xFF));
    outb(PIT_CHANNEL0_PORT, (uint8_t)((divisor >> 8) & 0xFF));

    printf("PIT initialised at %d Hz\n", TARGET_FREQUENCY);
}

void sleep_busy(uint32_t milliseconds)
{
    uint32_t start_tick    = get_current_tick();
    uint32_t ticks_to_wait = milliseconds * TICKS_PER_MS;
    uint32_t elapsed       = 0;

    while (elapsed < ticks_to_wait)
    {
        while (get_current_tick() == start_tick + elapsed)
            ;
        elapsed++;
    }
}

void sleep_interrupt(uint32_t milliseconds)
{
    uint32_t current_tick = get_current_tick();
    uint32_t end_ticks    = current_tick + milliseconds * TICKS_PER_MS;

    while (current_tick < end_ticks)
    {
        __asm__ volatile ("sti\n\t hlt");
        current_tick = get_current_tick();
    }
}
