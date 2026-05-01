
#include <drivers/audio/PCSPK.h>
#include <drivers/input/keyboard.h>
#include <kernel/pit.h>
#include <shell/music_player/frequencies.h>
#include <stdio.h>
#include <string.h>

static const note_t song_twinkle_twinkle[] = {
    C4(500), R(100),   C4(500),  R(100),  G4(500),  R(100),   G4(500), R(100),  A4(500),  R(100),
    A4(500), R(100),   G4(1000), F4(500), R(100),   F4(500),  R(100),  E4(500), R(100),   E4(500),
    R(100),  D4(500),  R(100),   D4(500), R(100),   C4(1000), G4(500), R(100),  G4(500),  R(100),
    F4(500), R(100),   F4(500),  R(100),  E4(500),  R(100),   E4(500), R(100),  D4(1000), G4(500),
    R(100),  G4(500),  R(100),   F4(500), R(100),   F4(500),  R(100),  E4(500), R(100),   E4(500),
    R(100),  D4(1000), C4(500),  R(100),  C4(500),  R(100),   G4(500), R(100),  G4(500),  R(100),
    A4(500), R(100),   A4(500),  R(100),  G4(1000), F4(500),  R(100),  F4(500), R(100),   E4(500),
    R(100),  E4(500),  R(100),   D4(500), R(100),   D4(500),  R(100),  C4(1000)};

static const note_t song_ode_to_joy[] = {
    E4(500), R(100),  E4(500), R(100),  F4(500), R(100),  G4(500), R(100),  G4(500), R(100),
    F4(500), R(100),  E4(500), R(100),  D4(500), C4(500), R(100),  C4(500), R(100),  D4(500),
    R(100),  E4(500), R(100),  E4(500), R(100),  D4(750), R(50),   D4(250), E4(500), R(100),
    E4(500), R(100),  F4(500), R(100),  G4(500), R(100),  G4(500), R(100),  F4(500), R(100),
    E4(500), R(100),  D4(500), C4(500), R(100),  C4(500), R(100),  D4(500), R(100),  E4(500),
    R(100),  D4(750), R(50),   C4(250), R(50),   C4(500)};

static const note_t song_scale[] = {C4(500), D4(500), E4(500), F4(500),
                                    G4(500), A4(500), B4(500), C5(500)};

static const note_t fifth_crescendo[] = {

    C1(200),  G1(200),  D2(200),  A2(200),  E3(200),  B3(200),
    FS4(200), CS5(200), GS5(200), DS6(200), AS6(200), F7(200),

    R(120),

    AS6(200), DS6(200), GS5(200), CS5(200), FS4(200), B3(200),
    E3(200),  A2(200),  D2(200),  G1(200),  C1(200),

    R(200),

    D1(200),  A1(200),  E2(200),  B2(200),  FS3(200), CS4(200),
    GS4(200), DS5(200), AS5(200), F6(200),  C7(200),

    R(120),

    G6(200),  C6(200),  F5(200),  AS4(200), DS4(200), GS3(200),
    CS3(200), FS2(200), B1(200),  E1(200),

};

static const note_t norwegian_anthem[] = {
    F4(1000),
    /* JA */ DS4(250),
    /* VI */ D4(500),
    /* EL- */ C4(500), /* SKER */
    AS3(500),
    /* DE- */ C4(500),
    /* TTE */ D4(500),
    /* LAN-  */ DS4(500), /* DET */
    F4(1000),
    /* SOM */ G4(250),
    /* DET */ F4(500),
    /* STI- */ DS4(500), /* GER */
    D4(2000),            /* FREM */

    G4(1000),
    /* FUR- */ F4(250),
    /* ET */ DS4(500),
    /* VÆR- */ D4(500), /* BITT */
    C4(500),
    /* OV- */ D4(500),
    /* ER */ DS4(500), /* VANN */
    F4(1000),
    /* ET */ R(250),
    F4(1000),
    /* MED */ G4(250), /* DE */
    G4(500),
    /* TU */ A4(500),
    /* SEN */ AS4(2000), /* HJEM */
};

static const note_t star_spangled_banner[] = {
    F4(300),   D4(300),  AS3(600),  // o o o
    D4(600),   F4(600),  AS4(1200), // can you see
    D5(600),   C5(150),  AS4(900),  //
    D4(600),   E4(600),  F4(1200),  F4(300),  F4(300),  D5(1200), C5(300),
    AS4(600),  A4(1200), G4(300),   A4(300),  AS4(600), AS4(600), F4(600),
    D4(600),   AS3(600), F4(450),   D4(150),  AS3(600), D4(600),  F4(600),
    AS4(1200), D5(450),  C5(150),   AS4(600), D4(600),  E4(600),  F4(2400), // enough i think
};


typedef struct {
  const char* name;
  const note_t* notes;
  size_t length;
} song_t;


static const song_t songs[] = {
    {        "twinkle", song_twinkle_twinkle, sizeof(song_twinkle_twinkle) / sizeof(note_t)},
    {     "ode_to_joy",      song_ode_to_joy,      sizeof(song_ode_to_joy) / sizeof(note_t)},
    {          "scale",           song_scale,           sizeof(song_scale) / sizeof(note_t)},
    {"fifth_crescendo",      fifth_crescendo,      sizeof(fifth_crescendo) / sizeof(note_t)},
    {         "norway",     norwegian_anthem,     sizeof(norwegian_anthem) / sizeof(note_t)},
    {            "usa", star_spangled_banner, sizeof(star_spangled_banner) / sizeof(note_t)},
};

#define NUM_SONGS (sizeof(songs) / sizeof(songs[0]))

static int check_interrupt(void) {
  if (!keyboard_has_key()) {
    return 0;
  }
  uint8_t key = 0;
  pop_key(&key);
  // return on esc or ctrl c
  if (key == 27 || key == 3) {
    return 1;
  }
  return 0;
}

// Plays a song
static void play_song(const song_t* song) {
  for (size_t i = 0; i < song->length; i++) {
    if (i % 10 == 0 && check_interrupt()) {
      printf("\nSong interrupted.\n");
      PCSPK_STOP();
      return;
    }
    if (song->notes[i].note > 0) {
      PCSPK_PLAY((uint16_t)song->notes[i].note);
      sleep_interrupt(song->notes[i].duration);
      PCSPK_STOP();
    } else {
      sleep_interrupt(song->notes[i].duration);
    }
    sleep_interrupt(50);
  }
}

int cmd_music_player(int argc, char** argv) {
  if (argc < 2 || strcmp(argv[1], "list") == 0) {
    printf("Available songs:\n");
    for (size_t i = 0; i < NUM_SONGS; i++) {
      printf("  %s\n", songs[i].name);
    }
    printf("Usage: music_player <song_name>\n");
    printf("Press ESC or Ctrl+C to stop.\n");
    return 0;
  }

  const char* song_name = argv[1];
  for (size_t i = 0; i < NUM_SONGS; i++) {
    if (strcmp(song_name, songs[i].name) == 0) {
      printf("Playing: %s (press ESC or Ctrl+C to stop)\n", songs[i].name);
      play_song(&songs[i]);
      printf("Done.\n");
      return 0;
    }
  }

  printf("Unknown song: %s\n", song_name);
  printf("Type 'music_player' to see available songs.\n");
  return 0;
}
