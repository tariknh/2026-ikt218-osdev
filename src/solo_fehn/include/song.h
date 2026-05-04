/*
 * song.h - Music data structures and player API
 *
 * A song is a flat array of Notes. Each note has a frequency in Hz
 * (0 = silence/rest) and a duration in milliseconds.
 *
 * SongPlayer wraps a function pointer so the rest of the kernel can call
 * player->play_song(&song) without caring about the implementation.
 */

#pragma once
#include <libc/stdint.h>
#include <libc/stddef.h>

typedef struct {
    uint32_t frequency;   /* Hz, 0 = silence (rest) */
    uint32_t duration;    /* ms */
} Note;

typedef struct {
    Note*   notes;
    size_t  note_count;
} Song;

typedef struct {
    void (*play_song)(Song* song);
} SongPlayer;

/* Allocates a SongPlayer on the heap with play_song wired to the default
   implementation. Free with regular free(). */
SongPlayer* create_song_player(void);

/* The default player implementation; you can also call this directly. */
void play_song_impl(Song* song);

/* Demonstration song data, defined in songs.c */
extern Note   music_1[];
extern size_t music_1_length;
