#include "pit.h"
#include "libc/stdint.h"
#include "libc/stdbool.h"

volatile uint32_t pit_ticks = 0;

static inline void outb(uint16_t port, uint8_t value) {
    __asm__ __volatile__("outb %0, %1" : : "a"(value), "Nd"(port));
}

uint32_t get_current_tick() {
    return pit_ticks;
}

void pit_irq_handler() {
    pit_ticks++;
    outb(PIC1_CMD_PORT, PIC_EOI);
}

void init_pit() {
    uint16_t divisor = DIVIDER;
    outb(PIT_CMD_PORT, 0x36);                           
    outb(PIT_CHANNEL0_PORT, (uint8_t)(divisor & 0xFF)); 
    outb(PIT_CHANNEL0_PORT, (uint8_t)(divisor >> 8));   
}

void sleep_busy(uint32_t milliseconds) {
    uint32_t start = pit_ticks;
    uint32_t ticks_to_wait = milliseconds * TICKS_PER_MS;
    while ((pit_ticks - start) < ticks_to_wait) {
       
    }
}


void sleep_interrupt(uint32_t milliseconds) {
    uint32_t start = pit_ticks;
    uint32_t ticks_to_wait = milliseconds * TICKS_PER_MS;
    while ((pit_ticks - start) < ticks_to_wait) {
        __asm__ __volatile__("sti; hlt");  
    }
}