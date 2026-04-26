#pragma once

#include "libc/stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint32_t frequency; /* Hz; 0 = rest/pause */
    uint32_t duration;  /* milliseconds */
} Note;

typedef struct {
    Note    *notes;
    uint32_t note_count;
} Song;

typedef struct {
    void (*play_song)(Song *song);
} SongPlayer;

void        play_song_impl(Song *song);
SongPlayer *create_song_player(void);

/* Defined in music.c */
extern Note     music_1[];
extern uint32_t music_1_count;

#ifdef __cplusplus
}
#endif
