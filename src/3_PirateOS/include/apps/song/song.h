#ifndef SONG_H
#define SONG_H

#include "frequencies.h"
#include "libc/system.h"

// Define a struct to represent a single musical note
typedef struct {
    uint32_t frequency; // The frequency of the note in Hz (e.g., A4 = 440 Hz)
    uint32_t duration;  // The duration of the note in milliseconds
} Note;

// Define a struct to represent a song
typedef struct {
    Note* notes;        // Pointer to an array of Note structs representing the song
    uint32_t length;    // The number of notes in the song
} Song;

// Define a struct to represent a song player
typedef struct {
    void (*play_song)(Song* song); // Function pointer to a function that plays a song
} SongPlayer;

// Allocate and initialize a song player.
SongPlayer* create_song_player();

// Play all notes in the provided song.
void play_song_impl(Song *song);

// Configure PIT channel 2 and start a tone.
void play_sound(uint32_t frequency);

// Stop any tone currently playing.
void stop_sound(void);

// Play one of the predefined songs by index.
void play_music(int song_index);

// Request current music playback to stop.
void stop_music(void);

#endif
