#ifndef SONG_PLAYER_H
#define SONG_PLAYER_H

#include "libc/stdint.h"
#include "song.h"
#include "../io/io.h"

/// Plays the first built-in song during boot.
void play_default_song(void);

/// Plays one of the built-in songs by index.
void play_song_by_index(uint32_t index);

/// Returns the number of built-in songs available for playback.
uint32_t get_song_count(void);

/// Small local helpers for port I/O used by the PC speaker.
uint8_t speaker_inb(uint16_t port);

void speaker_outb(uint16_t port, uint8_t value);

/// Bits 0 and 1 on port 0x61 must be enabled for audible output.
void enable_speaker(void);

void disable_speaker(void);

/// PIT channel 2 drives the PC speaker tone frequency.
void play_sound(uint32_t frequency);

void stop_sound(void);

/// Plays each note in sequence and uses the PIT sleep routine for timing.
void play_song_impl(Song *song);

void play_song(Song *song);

SongPlayer* create_song_player(void);


#endif
