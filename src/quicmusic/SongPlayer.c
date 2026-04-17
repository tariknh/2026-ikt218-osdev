#include "SongPlayer.h";
#include "pit.h";
#include "libc/stdio.h";
#include "libc/stdint.h"
#include "song.h";

static inline void outb(uint16_t port, uint8_t value)
{
    asm volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}
static inline uint8_t inb(uint16_t port)
{
    uint8_t v;
    asm volatile ("inb %1, %0" : "=a"(v) : "Nd"(port));
    return v;
}

void enable_speaker() {
    uint8_t controller = inb(PC_SPEAKER_PORT);
    controller |= 0x3; // Set bits 0 and 1
    outb(PC_SPEAKER_PORT, controller);
}

void disable_speaker() {
    uint8_t ctrl = inb(PC_SPEAKER_PORT);
    ctrl &= ~0x03;                    // clear bits 0 and 1
    outb(PC_SPEAKER_PORT, ctrl);
}

void play_sound(uint32_t frequency) {
    if (frequency == 0) {
        disable_speaker();
        return;
    }

     uint16_t divisor = (uint16_t)(PIT_BASE_FREQUENCY / frequency);
    if (divisor == 0) divisor = 1;   /* minimum divisor is 1 */
    /* 2. Command byte:
          bits 7‑6 = 10  → channel 2
          bits 5‑4 = 11  → lobyte/hibyte access
          bits 3‑1 = 011 → mode 3 (square‑wave)
          bit 0    = 0   → binary mode
       => 0b10110110 = 0xB6
    */
    outb(PIT_CMD_PORT, 0xB6);
    /* 3. Load divisor (low byte first, then high byte) */
    outb(PIT_CHANNEL2_PORT, (uint8_t)(divisor & 0xFF));      /* low  */
    outb(PIT_CHANNEL2_PORT, (uint8_t)(divisor >> 8));        /* high */
    /* 4. Enable the speaker (bits 0 and 1 of 0x61) */
    speaker_enable();
}

void stop_sound() {
    disable_speaker();
}