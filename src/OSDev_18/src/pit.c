#include <kernel/pit.h>
#include <kernel/interrupt.h>
#include <kernel/io.h>
#include <kernel/terminal.h>

// Keep tick ownership local so only the PIT handler updates the time base.
static volatile uint32_t pit_ticks = 0;

uint32_t GetCurrentTick(void){
    return pit_ticks;
}

static void PitIrqHandler(struct Registers* regs){
    (void)regs; // Required by interface, not needed in this implementation
    pit_ticks++;
}

void PitInitialize(void){
    uint16_t divisor = DIVIDER;

    RegisterInterruptHandler(IRQ0, PitIrqHandler);

    // 0x36 selects channel 0, low/high byte access, mode 3, binary counting.
    OutPortByte(PIT_CMD_PORT, 0x36);
    OutPortByte(PIT_CHANNEL0_PORT, (uint8_t)(divisor & 0xFF));
    OutPortByte(PIT_CHANNEL0_PORT, (uint8_t)((divisor >> 8) & 0xFF));
}

// Halt between timer interrupts instead of burning CPU in a tight loop.
void SleepInterrupt(uint32_t ticks_to_wait){
    uint32_t start_tick = GetCurrentTick();
    uint32_t end_tick = start_tick + ticks_to_wait;

    while(GetCurrentTick() < end_tick){
        __asm__ volatile ("sti; hlt");
    }
}

/* Busy-wait by repeatedly polling the tick counter until enough time passes. */
void SleepBusy(uint32_t milliseconds){ 
    uint32_t start_tick = GetCurrentTick();
    uint32_t ticks_to_wait = milliseconds * TICKS_PER_MS;

    while((GetCurrentTick() - start_tick) < ticks_to_wait){
        /* Spin until the timer handler advances pit_ticks far enough. */
    }
}

void SleepTest(){
    while (GetCurrentTick() < 15000) {
      TerminalWriteString("Sleeping with busy-waiting (HIGH CPU).\n");
      SleepBusy(1000);
      TerminalWriteString("Slept using busy-waiting.\n");

      TerminalWriteString("Sleeping with interrupts (LOW CPU).\n");
      SleepInterrupt(1000);
      TerminalWriteString("Slept using interrupts.\n");
  }
}
