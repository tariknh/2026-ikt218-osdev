#include "pit.h"
#include "ports.h"
#include "terminal.h"

static volatile uint32_t pit_ticks = 0;

void init_pit(void)
{
    uint16_t divider = (uint16_t)PIT_DIVIDER;

    pit_ticks = 0;

    port_byte_out(PIT_CMD_PORT, 0x36);
    port_byte_out(PIT_CHANNEL0_PORT, (uint8_t)(divider & 0xFF));
    port_byte_out(PIT_CHANNEL0_PORT, (uint8_t)((divider >> 8) & 0xFF));

    terminal_write("PIT initialized at 1000 Hz\n");
}

void pit_handle_tick(void)
{
    pit_ticks++;
}

uint32_t pit_get_ticks(void)
{
    return pit_ticks;
}

void sleep_busy(uint32_t milliseconds)
{
    uint32_t start_tick = pit_get_ticks();
    uint32_t ticks_to_wait = milliseconds * TICKS_PER_MS;
    uint32_t elapsed_ticks = 0;

    while (elapsed_ticks < ticks_to_wait) {
        while (pit_get_ticks() == start_tick + elapsed_ticks) {
        }

        elapsed_ticks++;
    }
}

void sleep_interrupt(uint32_t milliseconds)
{
    uint32_t current_tick = pit_get_ticks();
    uint32_t ticks_to_wait = milliseconds * TICKS_PER_MS;
    uint32_t end_tick = current_tick + ticks_to_wait;

    while (current_tick < end_tick) {
        asm volatile("sti");
        asm volatile("hlt");
        current_tick = pit_get_ticks();
    }
}
