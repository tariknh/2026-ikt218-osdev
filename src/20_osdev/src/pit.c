#include <libc/stdint.h>
#include "kernel/pit.h"

static volatile uint32_t pit_ticks = 0;

static inline void outb(uint16_t port, uint8_t value)
{
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

uint32_t pit_get_ticks(void)
{
    return pit_ticks;
}

void pit_tick(void)
{
    pit_ticks++;
}

void init_pit(void)
{
    uint16_t divisor = DIVIDER;

    outb(PIT_CMD_PORT, 0x36);
    outb(PIT_CHANNEL0_PORT, divisor & 0xFF);
    outb(PIT_CHANNEL0_PORT, (divisor >> 8) & 0xFF);
}

void sleep_busy(uint32_t milliseconds)
{
    uint32_t start_tick = pit_get_ticks();
    uint32_t ticks_to_wait = milliseconds * TICKS_PER_MS;

    while ((pit_get_ticks() - start_tick) < ticks_to_wait)
    {
    }
}

void sleep_interrupt(uint32_t milliseconds)
{
    uint32_t start_tick = pit_get_ticks();
    uint32_t ticks_to_wait = milliseconds * TICKS_PER_MS;

    while ((pit_get_ticks() - start_tick) < ticks_to_wait)
    {
        __asm__ volatile("sti");
        __asm__ volatile("hlt");
    }
}