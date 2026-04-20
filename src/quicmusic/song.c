#include "song.h"
#include "libc/stdio.h"
#include "kernel_memory.h"
#include "libc/stddef.h"
#include "SoundPlayer.h"

//convers an array of notes into a song object
Song* notes_to_song(Note* notes_arr[], uint32_t notes_arr_size) {
    uint32_t song_length = 0;
    
    for (uint32_t note_index = 0; note_index < notes_arr_size; note_index++) {
        song_length += notes_arr[note_index]->duration;
    }

    Song* song = malloc(sizeof(Song));
    if (!song) return NULL;
    song->length = song_length;
    song->notes = malloc(notes_arr_size * sizeof(Note*));
    // Memcpy isn't implemented.
    for (uint32_t i = 0; i < notes_arr_size; ++i) {
        song->notes[i] = *notes_arr[i];
    }
    return song;
};

void play_song_impl(Song* song) {
    enable_speaker();
    for (uint32_t i = 0; i < song->length; i++) {
        if (!song) return; // Check if the song pointer is null before proceeding

        Note* current_note = &song->notes[i];

        // play the current note
        play_sound(current_note->frequency);
        sleep_interrupt(current_note->duration);
    }
    disable_speaker();
}

//Allocates a songplayer in the heap
SongPlayer* create_song_player() {
    SongPlayer* player = malloc(sizeof(SongPlayer));
    if (!player) return NULL; //player is null if malloc fails

    player->play_song = 0; //Initialize the function pointer to null
    player->play_song = play_song_impl; //set the function pointer to the implementation of play_song

    return player;
};

//Plays a song
void play_song(Song* song) {
    SongPlayer* player = create_song_player();
    if (!player) return; // Check if the player was created successfully
    player->play_song(song);
    free(player); // Free the player after use
}

//Plays all songs
void play_music() {
    // How to play music
    Song songs[] = {
        {music_1, sizeof(music_1) / sizeof(Note)}
    };
    uint32_t n_songs = sizeof(songs) / sizeof(Song);

    SongPlayer* player = create_song_player();

    while(1) {
        for(uint32_t i = 0; i < n_songs; i++) {
            printf("Playing Song...\n");
            player->play_song(&songs[i]);
            printf("Finished playing the song.\n");
        }
    }
}