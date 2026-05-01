#include "song/song.h"

#include "pit.h"
#include "common.h"
#include "libc/stdio.h"

Note music_1[] = {
    {E5, 200}, {R, 80}, {E5, 200}, {R, 80}, {C5, 200}, {E5, 200},
    {G5, 300}, {R, 120}, {G4, 300}, {R, 200}
};

uint32_t music_1_len = sizeof(music_1) / sizeof(Note);

void enable_speaker(){
    uint8_t speaker_state = inb(PC_SPEAKER_PORT);
    if (speaker_state != (speaker_state | 3)) {
        // If bits 0 and 1 are not set, enable the speaker by setting bits 0 and 1 to 1
        outb(PC_SPEAKER_PORT, speaker_state | 3);
    }
}

void disable_speaker() {
    // Turn off the PC speaker
    uint8_t speaker_state = inb(PC_SPEAKER_PORT);
    outb(PC_SPEAKER_PORT, speaker_state & 0xFC);
}

void play_sound(uint32_t frequency) {
    if (frequency == 0) {
        return;
    }

    uint16_t divisor = (uint16_t)(PIT_BASE_FREQUENCY / frequency);

    //Set up the PIT
    outb(PIT_CMD_PORT, 0b10110110);
    outb(PIT_CHANNEL2_PORT, (uint8_t)(divisor & 0xFF));
    outb(PIT_CHANNEL2_PORT, (uint8_t)(divisor >> 8));

    //Enable the speaker by setting bits 0 and 1
    uint8_t speaker_state = inb(PC_SPEAKER_PORT);
    outb(PC_SPEAKER_PORT, speaker_state | 0x03);
}

void stop_sound(){
    //Stop the sound by disabling the gate to the speaker
    uint8_t speaker_state = inb(PC_SPEAKER_PORT);
    outb(PC_SPEAKER_PORT, speaker_state & ~0x03); // Clear bits 0 and 1
}

void play_song_impl(Song *song) {
    enable_speaker();

    for (uint32_t i = 0; i < song->length; i++) {
        Note* note = &song->notes[i];
        printf("Note: %d, Freq=%d, Sleep=%d\n", i, note->frequency, note->duration);

        if (note->frequency != 0) {
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

void play_music(void)
{
    Song s;
    s.notes = music_1;
    s.length = music_1_len;

    play_song_impl(&s);
}