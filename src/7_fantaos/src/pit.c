#include <pit.h>
#include <io.h>
#include <idt.h>
#include <pic.h>

// Incremented once per IRQ0 tick (1000 Hz - 1 ms per tick).
static volatile uint32_t tick_count = 0;

static void pit_handler(void) {
    tick_count++;
}

void init_pit(void) {
    // Command: channel 0, lobyte/hibyte access, mode 3 (square wave), binary.
    // 0x36 = 0b00110110
    outb(PIT_CMD_PORT, 0x36);

    // Send the 16-bit divisor as two bytes to channel 0.
    outb(PIT_CHANNEL0_PORT, (uint8_t)(DIVIDER & 0xFF));
    outb(PIT_CHANNEL0_PORT, (uint8_t)((DIVIDER >> 8) & 0xFF));

    irq_install_handler(0, pit_handler);
    pic_unmask_irq(0);
}

uint32_t get_tick(void) {
    return tick_count;
}

void sleep_busy(uint32_t milliseconds) {
    uint32_t start          = tick_count;
    uint32_t ticks_to_wait  = milliseconds * TICKS_PER_MS;
    uint32_t elapsed        = 0;

    while (elapsed < ticks_to_wait) {
        while (tick_count == start + elapsed)
            ; // busy wait, spins until the next tick arrives
        elapsed++;
    }
}

void sleep_interrupt(uint32_t milliseconds) {
    uint32_t end_tick = tick_count + (milliseconds * TICKS_PER_MS);

    while (tick_count < end_tick) {
        // Enable interrupts and halt; the PIT IRQ will wake us and increment tick_count.
        asm volatile("sti; hlt");
    }
}
