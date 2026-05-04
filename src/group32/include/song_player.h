#pragma once

#include "song/song.h" // Includes song and SongPlayer definitions

SongPlayer* create_song_player(); // Creates and returns a song player
void play_song_impl(Song* song); // Actual implementation for playing a song
void play_song(Song* song); // Wrapper function for playing a song
