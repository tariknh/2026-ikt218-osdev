#include <pit.h>
#include <io.h>
#include <terminal.h>
#include <libc/stdint.h>

static volatile uint32_t pit_ticks = 0;

void init_pit(void)
{
    uint32_t divisor = PIT_BASE_FREQUENCY / PIT_TARGET_FREQUENCY;

    /*
     * Command byte 0x36:
     *  - channel 0
     *  - access mode: low byte/high byte
     *  - mode 3: square wave generator
     *  - binary mode
     */
    outb(PIT_COMMAND_PORT, 0x36);

    /*
     * Sends divisor low byte, then high byte.
     */
    outb(PIT_CHANNEL0_PORT, (uint8_t)(divisor & 0xFF));
    outb(PIT_CHANNEL0_PORT, (uint8_t)((divisor >> 8) & 0xFF));

    terminal_write("PIT initialized at 1000 Hz\n");
}

void pit_handle_irq(void)
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

    while ((pit_get_ticks() - start_tick) < milliseconds) {
        /*
         * Busy waiting keeps CPU active while waiting.
         */
    }
}

void sleep_interrupt(uint32_t milliseconds)
{
    uint32_t start_tick = pit_get_ticks();

    while ((pit_get_ticks() - start_tick) < milliseconds) {
        /*
         * Enables interrupts and halt until next interrupt arrives.
         */
        __asm__ volatile ("sti");
        __asm__ volatile ("hlt");
    }
}
