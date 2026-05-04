#include "song/song.h"
#include "pit.h"
#include "common.h"
#include "libc/stdio.h"
#include "terminal.h"

// PC speaker control port bit masks
#define SPEAKER_GATE_BIT    0x01    // Bit 0: connects PIT channel 2 to speaker
#define SPEAKER_DATA_BIT    0x02    // Bit 1: enables speaker output

void enable_speaker() {
    uint8_t state = inb(PC_SPEAKER_PORT);
    outb(PC_SPEAKER_PORT, state | SPEAKER_GATE_BIT | SPEAKER_DATA_BIT);
}

void disable_speaker() {
    uint8_t state = inb(PC_SPEAKER_PORT);
    outb(PC_SPEAKER_PORT, state & ~(SPEAKER_GATE_BIT | SPEAKER_DATA_BIT));
}

void play_sound(uint32_t frequency) {
    if (frequency == 0) return; // R = rest, no sound

    uint32_t divisor = PIT_BASE_FREQUENCY / frequency;

    // Configure PIT channel 2: channel 2, lobyte/hibyte, mode 3 (square wave)
    outb(PIT_CMD_PORT, 0xB6);
    outb(PIT_CHANNEL2_PORT, (uint8_t)(divisor & 0xFF));
    outb(PIT_CHANNEL2_PORT, (uint8_t)((divisor >> 8) & 0xFF));
    enable_speaker();
}

void stop_sound() {
    uint8_t state = inb(PC_SPEAKER_PORT);
    outb(PC_SPEAKER_PORT, state & ~SPEAKER_DATA_BIT);
}


void play_song_impl(Song *song) {
    for (uint32_t i = 0; i < song->length; i++) {
        Note *note = &song->notes[i];
        if (note->frequency == 0) {
            stop_sound();
        }
        else {
            terminal_write("Note ");
            terminal_write_dec(i + 1);
            terminal_write(": freq ");
            terminal_write_dec(note->frequency);
            terminal_write("HZ, duration:");
            terminal_write_dec(note->duration);
            terminal_write("ms\n");
            //terminal_write("Note %u: freq=%u Hz, duration=%u ms\n",i, note->frequency, note->duration);
            play_sound(note->frequency);
            
        }
        sleep_interrupt(note->duration);
        stop_sound();
            
    }
    
    disable_speaker();
}

void play_song(Song *song) {
    play_song_impl(song);
}