#include "../../include/radio.h"
#include "../../include/keyboard.h"
#include "../../include/libc/stdio.h"
#include "../../include/menu.h"
#include "../lib/song/song.h"

// Defines all songs available in song.h
Song song_1 = {.notes = music_1,
               .length = sizeof(music_1) / sizeof(music_1[0])};

Song song_2 = {.notes = music_2,
               .length = sizeof(music_2) / sizeof(music_2[0])};

Song song_3 = {.notes = music_3,
               .length = sizeof(music_3) / sizeof(music_3[0])};

Song song_4 = {.notes = music_4,
               .length = sizeof(music_4) / sizeof(music_4[0])};

Song song_5 = {.notes = music_5,
               .length = sizeof(music_5) / sizeof(music_5[0])};

Song song_6 = {.notes = music_6,
               .length = sizeof(music_6) / sizeof(music_6[0])};

Song song_7 = {.notes = nasjonal_sangen,
               .length = sizeof(nasjonal_sangen) / sizeof(nasjonal_sangen[0])};

Song *songs[] = {[0] = &song_1, [1] = &song_2, [2] = &song_3, [3] = &song_4,
                 [4] = &song_5, [5] = &song_6, [6] = &song_7};

void radio_keyboard_handler(int scancode) {
  int test_key = get_key(scancode);
  if (test_key == -1) {
    return;
  }
  int key = test_key - 48;

  SongPlayer *songPlayer = create_song_player();
  printf("Playing song number: %d", key);
  Song *song_to_play = songs[key];
  Song song_spaced = add_space_between_notes(song_to_play, 50);
  songPlayer->play_song(&song_spaced);
  printf("Finished playing song number: %d", key);
  print_radio_menu();
}
