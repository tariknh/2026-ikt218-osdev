#include "pit.h"
#include "../interrupts/interrupts.h"

static volatile uint32_t pit_ticks = 0;

uint32_t get_current_tick(void) {
    return pit_ticks;
}

__attribute__((interrupt))
__attribute__((target("general-regs-only")))
void pit_irq_handler(struct interrupt_frame* frame) {
    (void)frame;
    pit_ticks++;
    outb(PIC1_CMD_PORT, PIC_EOI);
}

void init_pit(void) {
    uint16_t divisor = DIVIDER;

    outb(PIT_CMD_PORT, 0x36);
    outb(PIT_CHANNEL0_PORT, (uint8_t)(divisor & 0xFFU));
    outb(PIT_CHANNEL0_PORT, (uint8_t)((divisor >> 8) & 0xFFU));
}

void sleep_interrupt(uint32_t milliseconds) {
    uint32_t start = get_current_tick();
    uint32_t ticks_to_wait = milliseconds * TICKS_PER_MS;
    uint32_t end = start + ticks_to_wait;

    while (get_current_tick() < end) {
        __asm__ __volatile__("sti; hlt");
    }
}

void sleep_busy(uint32_t milliseconds) {
    uint32_t start = get_current_tick();
    uint32_t ticks_to_wait = milliseconds * TICKS_PER_MS;

    for (uint32_t i = 0; i < ticks_to_wait; i++) {
        while (get_current_tick() == start + i) {
            // busy wait
        }
    }
}
