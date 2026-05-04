#include "song.h"
#include "pit.h"
#include <libc/stdio.h>

#define PC_SPEAKER_PORT 0x61
#define PIT_CMD_PORT    0x43
#define PIT_CHANNEL2    0x42
#define PIT_FREQUENCY   1193180

  //write a byte to a hardware port
static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

//read a byte from a hardware port
static inline uint8_t inb(uint16_t port) {
    uint8_t value;
    __asm__ volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

//turn on the PC speaker by setting gate and data bits
void enable_speaker() {
    uint8_t state = inb(PC_SPEAKER_PORT);
    outb(PC_SPEAKER_PORT, state | 0x03);
}

//turn off the PC speaker by clearing gate and data bits
void disable_speaker() {
    uint8_t state = inb(PC_SPEAKER_PORT);
    outb(PC_SPEAKER_PORT, state & ~0x03);
}

//set PIT channel 2 to output the given frequency through the speaker
void play_sound(uint32_t frequency) {
    if (frequency == 0) return; //frequency 0 means rest, no sound
    uint32_t divisor = PIT_FREQUENCY / frequency;

    //tell PIT to use channel 2 in square wave mode
    outb(PIT_CMD_PORT, 0xB6);

    //send divisor as low byte then high byte
    outb(PIT_CHANNEL2, (uint8_t)(divisor & 0xFF));
    outb(PIT_CHANNEL2, (uint8_t)((divisor >> 8) & 0xFF));

    enable_speaker();
}

//stop sound by clearing the speaker data bit
void stop_sound() {
    uint8_t state = inb(PC_SPEAKER_PORT);
    outb(PC_SPEAKER_PORT, state & ~0x02);
}

//plays all notes in a song one by one
void play_song_impl(Song *song) {
    enable_speaker();
    for (uint32_t i = 0; i < song->length; i++) {
        printf("Playing note: freq=%d, dur=%d\n", song->notes[i].frequency, song->notes[i].duration);
        play_sound(song->notes[i].frequency);
        sleep_interrupt(song->notes[i].duration);
        stop_sound();
    }
    disable_speaker();
}