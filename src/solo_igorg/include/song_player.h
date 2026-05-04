#ifndef SONG_PLAYER_H
#define SONG_PLAYER_H

#include <song/song.h>

/*
 * Creates song player object.
 */
SongPlayer* create_song_player(void);

/*
 * Plays song by iterating through its notes.
 */
void play_song_impl(Song* song);

/*
 * Plays selected test song.
 */
void play_music(void);

#endif
