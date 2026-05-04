#include <stdint.h>
#include "kernel/pit.h"
#include "libc/system.h"
#include "irq.h"

void init_pit() {
    outb(PIT_CMD_PORT, 0x36);
    uint16_t divisor = (uint16_t)DIVIDER;
    outb(PIT_CHANNEL0_PORT, (uint8_t)(divisor & 0xFF));
    outb(PIT_CHANNEL0_PORT, (uint8_t)((divisor >> 8) & 0xFF));
    timer_ticks = 0;
}

void sleep_busy(uint32_t milliseconds) {
    uint32_t start = timer_ticks;
    while ((uint32_t)(timer_ticks - start) < milliseconds) {
        // busy wait
    }
}

void sleep_interrupt(uint32_t milliseconds) {
    uint32_t end = timer_ticks + milliseconds;
    while ((int32_t)(timer_ticks - end) < 0) {
        asm volatile("sti; hlt");
    }
}
