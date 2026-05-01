#include "kernel/pit.h"

#include "arch/i386/cpu/isr.h"
#include "arch/i386/cpu/ports.h"
#include "kernel/util.h"

// PIT (Programmable Interval Timer) related macros
#define PIT_CMD_PORT 0x43
#define PIT_CHANNEL0_PORT 0x40
#define PIT_CHANNEL1_PORT 0x41
#define PIT_CHANNEL2_PORT 0x42
#define PC_SPEAKER_PORT 0x61
#define PIT_DEFAULT_DIVISOR 0x4E20 // 20000, which gives about 18.2 Hz (1193180 / 20000)

// Channel 0 mod 2  (rate generator)
#define PIT_MODE_2 0x34

// IRQ0 related macros
#define PIC1_CMD_PORT 0x20
#define PIC1_DATA_PORT 0x20
#define PIC_EOI 0x20 /* End-of-interrupt command code */


#define PIT_BASE_FREQUENCY 1193182
#define TARGET_FREQUENCY 1000 // 1000 Hz
#define DIVIDER (PIT_BASE_FREQUENCY / TARGET_FREQUENCY)
#define TICKS_PER_MS (TARGET_FREQUENCY / 1000)


volatile uint32_t ticks = 0;

// increment global ticks counter and send EOI to PIC.
void timer_handler(registers_t* regs) {
  // make "atomic"
  __asm__ volatile("cli");
  ticks++;
  port_byte_out(PIC1_CMD_PORT, PIC_EOI); // acknowledge
  __asm__ volatile("sti");
}

// Configure pit channel 0 as a periodic timer at TARGET_FREQUENCY hz
// connected to IRQ0, and install timer handler in IDT
void init_pit() {
  uint32_t divisor = CLAMP(DIVIDER, 1, 65536);

  // Must be atomic
  __asm__ volatile("cli");
  port_byte_out(PIT_CMD_PORT, PIT_MODE_2);
  port_byte_out(PIT_CHANNEL0_PORT, divisor & 0xFF);
  port_byte_out(PIT_CHANNEL0_PORT, divisor >> 8 & 0xFF);
  __asm__ volatile("sti");

  register_interrupt_handler(IRQ0, timer_handler);
}
uint32_t get_current_tick() {
  // must be atomic
  __asm__ volatile("cli");
  uint32_t tick = ticks;
  __asm__ volatile("sti");
  return tick;
}

void sleep_interrupt(uint32_t milliseconds) {
  uint32_t start_tick = get_current_tick();
  uint32_t ticks_to_wait = milliseconds * TICKS_PER_MS;

  __asm__ volatile("sti");
  while (get_current_tick() - start_tick < ticks_to_wait)
    __asm__ volatile("hlt");
}

void sleep_busy(uint32_t milliseconds) {
  uint32_t start_tick = get_current_tick();
  uint32_t ticks_to_wait = milliseconds * TICKS_PER_MS;

  __asm__ volatile("sti");
  while (get_current_tick() - start_tick < ticks_to_wait)
    ;
}
