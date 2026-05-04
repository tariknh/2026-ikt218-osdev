#include <libc/stdint.h>

#include "pit.h"
#include "io.h"
#include "terminal.h"

#define PIT_CMD_PORT 0x43
#define PIT_CHANNEL0_PORT 0x40
#define PIC1_DATA_PORT 0x21

static volatile uint32_t pit_ticks = 0;

void pit_on_irq0(void) {
    pit_ticks++;
}

uint32_t pit_get_ticks(void) {
    return pit_ticks;
}

void init_pit(void) {
    uint16_t divisor = (uint16_t)PIT_DIVISOR;

    /*
     * 0x36 = channel 0, lobyte/hibyte, mode 3 (square wave), binary mode.
     * Channel 0 drives IRQ0, which we use as our millisecond tick source.
     */
    outb(PIT_CMD_PORT, 0x36);
    outb(PIT_CHANNEL0_PORT, (uint8_t)(divisor & 0xFF));
    outb(PIT_CHANNEL0_PORT, (uint8_t)((divisor >> 8) & 0xFF));

    /* Ensure IRQ0 is unmasked on the master PIC. */
    uint8_t mask = inb(PIC1_DATA_PORT);
    mask &= (uint8_t)~0x01;
    outb(PIC1_DATA_PORT, mask);

    terminal_write("PIT initialized at ");
    terminal_write_dec(PIT_TARGET_FREQUENCY);
    terminal_write(" Hz.\n");
}

void sleep_busy(uint32_t milliseconds) {
    uint32_t start = pit_get_ticks();
    while ((uint32_t)(pit_get_ticks() - start) < milliseconds) {
        __asm__ volatile ("pause");
    }
}

void sleep_interrupt(uint32_t milliseconds) {
    uint32_t start = pit_get_ticks();
    while ((uint32_t)(pit_get_ticks() - start) < milliseconds) {
        __asm__ volatile ("sti; hlt");
    }
}