#include "drivers/audio/PCSPK.h"

#include <arch/i386/cpu/ports.h>

#define PIT_MODE_REG 0x43
#define PIT_CH2_DATA 0x42
#define PIT_SPEAKER_GATE_PORT 0x61
#define PIT_SQUARE_WAVE_MODE \
  0xB6                         // select channel 2, access mode: lobyte hibyte, squaremode, 16-bit
#define PIT_LATCH_CH2_CMD 0xB0 // Latch CH2 status (bits 7-6=10, bits 5-6 (latch )=0)

#define SPEAKER_ENABLE_BITS 0x03  // Bit 0: PIT CH2 gate, Bit 1: Speaker enable
#define SPEAKER_DISABLE_MASK 0xFC // Clear bits 0-1

#define PIT_CLOCK_HZ 1193182UL
#define MAX_PLAY_FREQ_HZ 15000UL
#define MIN_PLAY_FREQ_HZ 1UL

void PCSPK_init() {
  PCSPK_STOP();
}

// From https://wiki.osdev.org/PC_Speaker
void PCSPK_PLAY(uint32_t frequency) {
  if (frequency < MIN_PLAY_FREQ_HZ || frequency > MAX_PLAY_FREQ_HZ) {
    return;
  }
  uint32_t Div = PIT_CLOCK_HZ / frequency;
  port_byte_out(PIT_MODE_REG, PIT_SQUARE_WAVE_MODE);
  port_byte_out(PIT_CH2_DATA, (uint8_t)Div);
  port_byte_out(PIT_CH2_DATA, (uint8_t)(Div >> 8));
  uint8_t tmp = port_byte_in(PIT_SPEAKER_GATE_PORT);
  port_byte_out(PIT_SPEAKER_GATE_PORT, tmp | SPEAKER_ENABLE_BITS);
}

void PCSPK_STOP() {
  uint8_t tmp = port_byte_in(PIT_SPEAKER_GATE_PORT) & SPEAKER_DISABLE_MASK;
  port_byte_out(PIT_SPEAKER_GATE_PORT, tmp);
}
