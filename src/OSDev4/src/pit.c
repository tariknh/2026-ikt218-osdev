#include "pit.h"
#include "io.h"
#include "irq.h"

static volatile uint32_t tick_count = 0;

uint32_t get_current_tick(void)
{
    return tick_count;
}

static void pit_tick_handler(registers_t* regs)
{
    (void)regs;
    tick_count++;
}

void init_pit(void)
{
    outb(PIT_CMD_PORT, 0x36);
    outb(PIT_CHANNEL0_PORT, (uint8_t)(DIVIDER & 0xFF));
    outb(PIT_CHANNEL0_PORT, (uint8_t)((DIVIDER >> 8) & 0xFF));
    irq_register_handler(0, pit_tick_handler);
}

void sleep_busy(uint32_t milliseconds)
{
    uint32_t start = tick_count;
    uint32_t ticks = milliseconds * TICKS_PER_MS;
    while ((tick_count - start) < ticks) {
    }
}

void sleep_interrupt(uint32_t milliseconds)
{
    uint32_t start = tick_count;
    uint32_t ticks = milliseconds * TICKS_PER_MS;
    while ((tick_count - start) < ticks) {
        __asm__ volatile ("sti\nhlt");
    }
}
