#include "pit.h"
#include "irq.h"
#include "terminal.h"
#include "libc/stdint.h"

/* We need port I/O for programming the PIT */
static inline void outb(uint16_t port, uint8_t val)
{
    asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

/* Global tick counter, increased by IRQ0 */
static volatile uint32_t timer_ticks = 0;

/* Timer interrupt callback for IRQ0 */
static void pit_callback(struct isr_frame* frame)
{
    (void)frame;
    timer_ticks++;
}

/* Helper to read current tick count */
static uint32_t get_current_tick(void)
{
    return timer_ticks;
}

/* Initialize PIT to 1000 Hz */
void init_pit(void)
{
    uint16_t divisor = DIVIDER;

    /* Register PIT handler on IRQ0 */
    irq_register_handler(0, pit_callback);

    /* Command byte: channel 0, lobyte/hibyte, mode 3, binary */
    outb(PIT_CMD_PORT, 0x36);

    /* Send divisor */
    outb(PIT_CHANNEL0_PORT, (uint8_t)(divisor & 0xFF));
    outb(PIT_CHANNEL0_PORT, (uint8_t)((divisor >> 8) & 0xFF));

    printf("PIT initialized at %d Hz\n", TARGET_FREQUENCY);
}

/* Sleep by busy-waiting */
void sleep_busy(uint32_t milliseconds)
{
    uint32_t start_tick = get_current_tick();
    uint32_t ticks_to_wait = milliseconds * TICKS_PER_MS;

    while ((get_current_tick() - start_tick) < ticks_to_wait) {
        /* Busy wait */
    }
}

/* Sleep using interrupts + hlt */
void sleep_interrupt(uint32_t milliseconds)
{
    uint32_t start_tick = get_current_tick();
    uint32_t ticks_to_wait = milliseconds * TICKS_PER_MS;

    while ((get_current_tick() - start_tick) < ticks_to_wait) {
        asm volatile("sti");
        asm volatile("hlt");
    }
}