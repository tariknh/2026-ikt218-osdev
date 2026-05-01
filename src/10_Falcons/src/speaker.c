#include <stdint.h>
#include <kernel/io.h>
#include <kernel/speaker.h>

#define PIT_CMD_PORT        0x43
#define PIT_CHANNEL2_PORT   0x42
#define PC_SPEAKER_PORT     0x61
#define PIT_BASE_FREQUENCY  1193180

void enable_speaker(void)
{
    uint8_t value = inb(PC_SPEAKER_PORT);
    outb(PC_SPEAKER_PORT, value | 0x03);
}

void disable_speaker(void)
{
    uint8_t value = inb(PC_SPEAKER_PORT);
    outb(PC_SPEAKER_PORT, value & 0xFC);
}

void play_sound(uint32_t frequency)
{
    if (frequency == 0) {
        stop_sound();
        return;
    }

    uint32_t divisor = PIT_BASE_FREQUENCY / frequency;

    outb(PIT_CMD_PORT, 0xB6);
    outb(PIT_CHANNEL2_PORT, (uint8_t)(divisor & 0xFF));
    outb(PIT_CHANNEL2_PORT, (uint8_t)((divisor >> 8) & 0xFF));

    enable_speaker();
}

void stop_sound(void)
{
    uint8_t value = inb(PC_SPEAKER_PORT);
    outb(PC_SPEAKER_PORT, value & 0xFC);
}
