#include "../../include/kernel/pit.h"
#include "../../include/io.h"
#include "../../include/irq.h"
#include "../../include/libc/stdio.h"

// Counter for the amount of PIT ticks since init_pit() was called.
static volatile uint32_t tick = 0;

static uint32_t get_current_tick() { return tick; }

// Handler for IRQ0 called everytime the PIT triggers a hardware interrupt
static void pit_irq_handler(registers_t *regs) { tick++; }

// Initialise the PIT to fire at TARGET_FREQUENCY.
void init_pit() {
  irq_register_handler(0, pit_irq_handler);

  // Bit 7 - 6 = 00 = channel 0
  // Bit 5 - 4 = 11 = access mode: lobyte/hibyte
  // Bit 3 - 1 = 011 = square wave generator
  // bit 0 = 0 = 16-bit binary mode
  // 0011 0110 = 0x36
  uint16_t divisor = DIVIDER;
  outb(PIT_CMD_PORT, 0x36);
  outb(PIT_CHANNEL0_PORT, divisor & 0xFF);
  outb(PIT_CHANNEL0_PORT, (divisor >> 8) & 0xFF);
}

// Busy sleeping where the CPU runs useless while loop while waiting.
void sleep_busy(uint32_t milliseconds) {
  uint32_t start_tick = get_current_tick();
  uint32_t ticks_to_wait = milliseconds * TICKS_PER_MS;
  uint32_t elapsed_ticks = 0;

  while (elapsed_ticks < ticks_to_wait) {
    while (get_current_tick() == start_tick + elapsed_ticks) {
    }
    elapsed_ticks++;
  }
}

// Interrupt driven sleep where we allow interrupts with `sti` instruction,
// and put the CPU in a low resource usage mode waiting for hardware interrupt
// using the `hlt` instruction
void sleep_interrupt(uint32_t milliseconds) {
  uint32_t current_tick = get_current_tick();
  uint32_t ticks_to_wait = milliseconds * TICKS_PER_MS;
  uint32_t end_ticks = current_tick + ticks_to_wait;

  while (current_tick < end_ticks) {
    asm volatile("sti");
    asm volatile("hlt");
    current_tick = get_current_tick();
  }
}
