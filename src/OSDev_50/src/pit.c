#include "pit.h"
#include "interrupts.h"
#include "common.h"
#include <libc/stddef.h>  // NULL

static volatile uint32_t ticks = 0;

static void pit_irq_handler(registers_t* regs, void* context)
{
    (void)regs;
    (void)context;
    ticks++;
}

void init_pit()
{
    register_irq_handler(0, pit_irq_handler, NULL);

    outb(PIT_CMD_PORT, 0x36); // channel 0, lobyte/hibyte, mode 3, binary

    uint16_t divisor = (uint16_t)(PIT_BASE_FREQUENCY / TARGET_FREQUENCY);
    uint8_t l_div = (uint8_t)(divisor & 0xFF);
    uint8_t h_div = (uint8_t)((divisor >> 8) & 0xFF);

    outb(PIT_CHANNEL0_PORT, l_div);
    outb(PIT_CHANNEL0_PORT, h_div);
}

void sleep_interrupt(uint32_t milliseconds)
{
    uint32_t end_ticks = ticks + (milliseconds * TICKS_PER_MS);

    while (ticks < end_ticks) {
        asm volatile("sti; hlt");
    }
}

void sleep_busy(uint32_t milliseconds)
{
    uint32_t end_ticks = ticks + (milliseconds * TICKS_PER_MS);

    while (ticks < end_ticks) {
        // busy wait
    }
}