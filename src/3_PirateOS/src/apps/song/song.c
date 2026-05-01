#include "apps/song/song.h"
#include "apps/song/song_data.h"

#include "kernel/pit.h"
#include "memory/heap.h"
#include "arch/i386/io.h"
#include "libc/stdio.h"

// Stop flag for active playback
static volatile int music_stop_flag = 0;

// Wait for note duration and stop early if ESC was pressed
// Returns 1 when playback should stop and returns 0 when full wait completed
static int wait_note_or_stop(uint32_t milliseconds)
{
    // Tick value when this wait starts
    uint32_t start_ticks;

    // Number of PIT ticks to wait for this note
    uint32_t ticks_to_wait;

    // If note length is 0 ms, skip waiting and just return current stop state
    if (milliseconds == 0) {
        return music_stop_flag;
    }

    // Read current global tick counter
    start_ticks = pit_get_ticks();

    // Convert milliseconds to PIT ticks
    ticks_to_wait = milliseconds * TICKS_PER_MS;

    // Loop until enough ticks have passed
    while ((pit_get_ticks() - start_ticks) < ticks_to_wait) {
        // Exit immediately when stop was requested by ESC
        if (music_stop_flag) {
            return 1;
        }

        // Enable interrupts and sleep until next interrupt
        // This avoids busy waiting and keeps CPU usage low
        asm volatile("sti\nhlt");
    }

    // Return current stop flag state when wait is done
    return music_stop_flag;
}

// Turn speaker output on by setting bit 0 and bit 1
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

// Turn speaker output off by clearing bit 0 and bit 1
void disable_speaker() {
    uint8_t speaker_state = inb(PC_SPEAKER_PORT);
    outb(PC_SPEAKER_PORT, speaker_state & 0xFC);
}

// Program PIT channel 2 to generate one tone frequency
void play_sound(uint32_t frequency) {
    if (frequency == 0) {
        return;
    }

    uint16_t divisor = (uint16_t)(PIT_BASE_FREQUENCY / frequency);

    // Set up the PIT
    outb(PIT_CMD_PORT, 0b10110110); 
    outb(PIT_CHANNEL2_PORT, (uint8_t)(divisor & 0xFF));
    outb(PIT_CHANNEL2_PORT, (uint8_t)(divisor >> 8));

    // Enable speaker after frequency is loaded
    enable_speaker();
}

// Stop current tone
void stop_sound(){
    disable_speaker();
}

// Core loop that plays all notes in order
void play_song_impl(Song *song) {
    if (!song || !song->notes || song->length == 0) {
        return;
    }

    // Make sure speaker is ready before first note
    enable_speaker();

    for (uint32_t i = 0; i < song->length; i++) {
        // Break if stop was requested
        if (music_stop_flag) {
            break;
        }

        Note* note = &song->notes[i];

        // Frequency 0 means pause
        if (note->frequency == 0) {
            stop_sound();
            if (wait_note_or_stop(note->duration)) {
                break;
            }
            continue;
        }

        // Normal note playback
        play_sound(note->frequency);
        if (wait_note_or_stop(note->duration)) {
            break;
        }
    }
    stop_sound();
}

// Function for playing a song (sequence of notes)
void play_song(Song *song) {
    music_stop_flag = 0;  // Reset flag
    play_song_impl(song);
}

SongPlayer* create_song_player() {
    // Allocate player object and attach play callback
    SongPlayer* player = (SongPlayer*)malloc(sizeof(SongPlayer));
    if (player) {
        player->play_song = play_song;
    }
    return player;
}

// Function to play a specific song by index
void play_music(int song_index) {
    // Validate index before accessing array
    if (song_index < 0 || (uint32_t)song_index >= available_song_count) {
        printf("Invalid song index. Available songs: 0-%u\n", available_song_count - 1);
        return;
    }

    // Create a song player
    SongPlayer* player = create_song_player();
    if (!player) {
        printf("Failed to create song player.\n");
        return;
    }

    printf("Playing song %d...\n", song_index);
    
    // Play the selected song
    player->play_song(&available_songs[song_index]);
    
    printf("Finished song %d.\n", song_index);

    // Free allocated memory
    free(player);
}

void stop_music(void) {
    // Set stop flag and silence speaker
    music_stop_flag = 1;
    stop_sound();
}
