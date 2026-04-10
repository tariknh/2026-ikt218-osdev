#include "pit.h"

static volatile uint32_t g_pit_ticks = 0;

static inline void outb(uint16_t port, uint8_t value) {
    asm volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

void init_pit(void) {
    uint16_t divisor = (uint16_t)DIVIDER;

    //Channel 0, lobyte/hibyte, mode 2 (rate generator), binary mode.
    outb(PIT_CMD_PORT, 0x34);
    outb(PIT_CHANNEL0_PORT, (uint8_t)(divisor & 0xFF));
    outb(PIT_CHANNEL0_PORT, (uint8_t)((divisor >> 8) & 0xFF));
}

void pit_on_tick(void) {
    ++g_pit_ticks;
}

uint32_t get_current_tick(void) {
    return g_pit_ticks;
}

void sleep_busy(uint32_t milliseconds) {
    uint32_t start_tick = get_current_tick();
    uint32_t ticks_to_wait = milliseconds * TICKS_PER_MS;

    while ((uint32_t)(get_current_tick() - start_tick) < ticks_to_wait) {
        asm volatile("pause");
    }
}

void sleep_interrupt(uint32_t milliseconds) {
    uint32_t start_tick = get_current_tick();
    uint32_t ticks_to_wait = milliseconds * TICKS_PER_MS;

    while ((uint32_t)(get_current_tick() - start_tick) < ticks_to_wait) {
        asm volatile("sti; hlt");
    }
}
