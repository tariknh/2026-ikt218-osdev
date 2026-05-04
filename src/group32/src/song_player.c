#include "song_player.h"
#include "song/song.h"
#include "pit.h"
#include "ports.h"
#include "memory.h"
#include "libc/stdint.h"

// PIT and speaker ports
#define PIT_COMMAND_PORT   0x43
#define PIT_CHANNEL2_PORT  0x42
#define PC_SPEAKER_PORT    0x61
// Enables the PC speaker
static void enable_speaker() {
    uint8_t value = inb(PC_SPEAKER_PORT);
    // Turn on speaker bits
    if ((value & 0x03) != 0x03) {
        outb(PC_SPEAKER_PORT, value | 0x03);
    }
}
// Disables the PC speaker
static void disable_speaker() {
    uint8_t value = inb(PC_SPEAKER_PORT);
    outb(PC_SPEAKER_PORT, value & 0xFC);    // Clear speaker bits

}
// Plays one frequency
static void play_sound(uint32_t frequency) {
    // Do nothing for rest notes
    if (frequency == 0) {
        return;
    }

    uint32_t divisor = PIT_BASE_FREQUENCY / frequency;     // Calculate PIT divisor

    outb(PIT_COMMAND_PORT, 0xB6);     // Configure PIT channel 2 for speaker
    outb(PIT_CHANNEL2_PORT, (uint8_t)(divisor & 0xFF));     // Send divisor low byte
    outb(PIT_CHANNEL2_PORT, (uint8_t)((divisor >> 8) & 0xFF));     // Send divisor high byte

    enable_speaker();     // Enable speaker output
}

// Stops the current sound
static void stop_sound() {
    uint8_t value = inb(PC_SPEAKER_PORT);
    outb(PC_SPEAKER_PORT, value & 0xFC);     // Disable speaker output
}

// Plays a full song
void play_song_impl(Song* song) {
    // Validate song
    if (!song || !song->notes || song->length == 0) {
        return;
    }

    enable_speaker();     // Enable speaker

    // Play each note
    for (uint32_t i = 0; i < song->length; i++) {
        Note note = song->notes[i];
        // Rest note
        if (note.frequency == 0) {
            stop_sound();
        } else {
            play_sound(note.frequency);
        }

        pit_sleep_ms(note.duration);         // Wait for note duration
        stop_sound();         // Stop sound between notes
    }

    disable_speaker();     // Disable speaker after song
}

// Wrapper for playing a song
void play_song(Song* song) {
    play_song_impl(song);
}

// Creates a SongPlayer object
SongPlayer* create_song_player() {
    SongPlayer* player = (SongPlayer*)malloc(sizeof(SongPlayer));     // Allocate memory for player
    // Return null if allocation failed
    if (!player) {
        return 0;
    }

    player->play_song = play_song;     // Set function pointer
    return player;
}