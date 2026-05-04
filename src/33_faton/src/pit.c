#include <pit.h>
#include <irq.h>
#include <isr.h>
#include <monitor.h>

static uint32_t pit_ticks = 0;

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ __volatile__("outb %0, %1" : : "a"(val), "Nd"(port));
}

static void pit_callback(registers_t* regs, void* ctx) {
    pit_ticks++;
}

void init_pit(void) {
    register_irq_handler(IRQ0, pit_callback, 0);

    uint32_t divisor = PIT_BASE_FREQUENCY / TARGET_FREQUENCY;

    outb(PIT_CMD_PORT,      0x36);
    outb(PIT_CHANNEL0_PORT, (uint8_t)(divisor & 0xFF));
    outb(PIT_CHANNEL0_PORT, (uint8_t)((divisor >> 8) & 0xFF));

    pit_ticks = 0;
    printf("PIT initialized at %d Hz\n", TARGET_FREQUENCY);
}

uint32_t get_current_tick(void) {
    return pit_ticks;
}

void sleep_busy(uint32_t milliseconds) {
    uint32_t start_tick   = get_current_tick();
    uint32_t ticks_to_wait = milliseconds * TICKS_PER_MS;
    uint32_t elapsed      = 0;

    while (elapsed < ticks_to_wait) {
        while (get_current_tick() == start_tick + elapsed);
        elapsed++;
    }
}

void sleep_interrupt(uint32_t milliseconds) {
    uint32_t end_tick = get_current_tick() + milliseconds;
    while (get_current_tick() < end_tick) {
        __asm__ __volatile__("sti");
        __asm__ __volatile__("hlt");
    }
}