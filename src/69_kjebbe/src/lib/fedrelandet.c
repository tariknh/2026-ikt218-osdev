#include "../../include/kernel/memory.h"
#include "../../include/kernel/pit.h"
#include "../../include/keyboard.h"
#include "../../include/libc/stdio.h"
#include "../../include/matrix.h"
#include "./song/song.h"

Song nasjonal = {.notes = nasjonal_sangen,
                 .length =
                     sizeof(nasjonal_sangen) / sizeof(nasjonal_sangen[0])};

// Prints Norwegian flag and plays Norwegian national anthem
void hedre_fedrelandet() {
  SongPlayer *songPlayer = create_song_player();
  Song song_spaced = add_space_between_notes(&nasjonal, 50);
  clearTerminal();
  ja_vi_elsker();
  songPlayer->play_song(&song_spaced);
  free(song_spaced.notes);
}

void fedrelandet_keyboard_handler(int scancode) {
  int key = get_key(scancode);
  if (key == 'j') {
    ja_vi_elsker();
  }
  if (key == 'h') {
    hedre_fedrelandet();
  }
}
