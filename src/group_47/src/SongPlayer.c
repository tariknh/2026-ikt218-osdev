#include "song.h"
#include "pit.h"
#include "io.h"
#include "libc/stdint.h"
#include "terminal.h"
#include "memory.h"

void enable_speaker(){
    // Read the current state of the PC speaker control register
    uint8_t speaker_state = inb(PC_SPEAKER_PORT);
    /*
    Bit 0: Speaker gate
            0: Speaker disabled
            1: Speaker enabled
    Bit 1: Speaker data
            0: Data is not passed to the speaker
            1: Data is passed to the speaker
    */
    // Check if bits 0 and 1 are not set (0 means that the speaker is disabled)
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
    if (frequency == 0) { // If the frequency is 0, we want to stop the sound (rest)
        return;
    }

    uint16_t divisor = (uint16_t)(PIT_BASE_FREQUENCY / frequency);

    // Set up the PIT to generate a square wave at the desired frequency on channel 2
    outb(PIT_CMD_PORT, 0b10110110); 
    outb(PIT_CHANNEL2_PORT, (uint8_t)(divisor & 0xFF)); // Send the low byte of the divisor to channel 2
    outb(PIT_CHANNEL2_PORT, (uint8_t)(divisor >> 8)); // Send the high byte of the divisor to channel 2

    uint8_t speaker_state = inb(PC_SPEAKER_PORT);
    if (!(speaker_state & 0x02)) {
        outb(PC_SPEAKER_PORT, speaker_state | 0x02);
    }
}

// Function to stop the sound by disabling the speaker gate
void stop_sound() {
    uint8_t speaker_state = inb(PC_SPEAKER_PORT);
    outb(PC_SPEAKER_PORT, speaker_state & 0xFC);
}

// Function to play a song by iterating through its notes with their destired frecuencies and durations
void play_song_impl(Song *song) {
    enable_speaker();
    for (uint32_t i = 0; i < song->length; i++) {
        Note* note = &song->notes[i];

        if (note->frequency ==0) {
            stop_sound();

        }else {
            play_sound(note->frequency);
        }
        sleep_interrupt(note->duration);
    }
    stop_sound();
    disable_speaker();
}

void play_song(Song *song) {
    play_song_impl(song);
}

// Function to create a new SongPlayer instance
SongPlayer* create_song_player() {
    static SongPlayer player;
    player.play_song = play_song_impl;
    return &player;
}

// Function to play music, which will be called from the kernel main loop. It will play the songs in an infinite loop.
void play_music() {
    // How to play music
    Song songs[] = {
        {music_4, sizeof(music_4) / sizeof(Note)}
    };
    uint32_t n_songs = sizeof(songs) / sizeof(Song);

    SongPlayer* player = create_song_player(); // Create a SongPlayer instance

    while(1) {
        for(uint32_t i = 0; i < n_songs; i++) {
            printf("Playing Song...\n");
            player->play_song(&songs[i]);
            printf("Finished playing the song.\n");
        }
    }
}