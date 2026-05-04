# PC Speaker Music Playback

## What we added

We added boot-time music playback using the PC speaker (`PCSPK`) and the PIT.
Relevant files:

- `src/group_29/src/sound/song.h`
- `src/group_29/src/sound/song_player.h`
- `src/group_29/src/sound/song_player.c`
- `src/group_29/src/pit/pit.h`
- `src/group_29/src/pit/pit.c`
- `src/group_29/src/kernel.c`
- `src/group_29/CMakeLists.txt`

## How it works

### 1. Song data

`src/group_29/src/sound/song.h` contains:

- `Note`
  - `frequency`
  - `duration`
- `Song`
  - pointer to notes
  - song length
- several built-in note arrays like `music_1`, `starwars_theme`, etc.
- a `built_in_songs[]` table so songs can later be selected by index
This means the music itself is just encoded as arrays of `{frequency, duration}`.

### 2. PIT timing

`src/group_29/src/pit/pit.c` already provided timing support.
Important parts:

- `init_pit()` configures PIT channel 0 for regular timer interrupts
- `pit_irq_handler(...)` increments the global tick counter
- `sleep_interrupt(ms)` waits using PIT ticks
Music playback uses this sleep function so each note lasts for the correct duration.

### 3. PC speaker playback

The actual speaker logic is in `src/group_29/src/sound/song_player.c`.
Important functions:

- `enable_speaker()`
  - reads port `0x61`
  - enables the PC speaker control bits
- `disable_speaker()`
  - disables the speaker after playback finishes
- `play_sound(frequency)`
  - computes PIT divisor from the requested note frequency
  - programs PIT channel 2
  - enables the speaker so the note becomes audible
- `stop_sound()`
  - stops the current tone
- `play_song_impl(song)`
  - loops through the notes
  - plays each frequency
  - waits for the duration
  - stops the note
- `play_default_song()`
  - currently plays the first built-in song
- `play_song_by_index(index)`
  - helper for future keyboard-triggered song selection

### 4. Boot integration

In `src/group_29/src/kernel.c`, after PIT initialization, we now call:

```c
play_default_song();
So the song starts automatically during boot.
5. Build integration
src/group_29/CMakeLists.txt was updated to compile:
- src/sound/song_player.c
Without that, the new sound code would not be linked into the kernel.
Why the boot pause mattered
When testing, the kernel paused for a few seconds on boot even when no sound was heard.
That told us:
- play_default_song() was being called
- PIT timing was active
- the song loop was running
So the code path was working, even if audio was not audible on every setup.
Cross-platform test result
We tested on different machines:
- Windows teammate: sound worked
- Arch host inside devcontainer: no audible sound
- Arch host running QEMU directly on the host: sound worked
This strongly suggests the main issue is not the kernel sound code, but audio passthrough from the Linux devcontainer to the host audio system.
Practical conclusion
Shared team workflow
- build inside the devcontainer
- run normally with the existing team scripts for standard testing
Arch sound-testing workflow
For reliable sound on Arch, QEMU should be run on the host, not inside the container.
That is because the host-run QEMU successfully produced audible PC speaker output, while container-run QEMU did not create a host audio stream.
Future extension
The code was structured so keyboard-triggered music can be added later without reworking the playback system.
The intended next step would be:
- leave all playback logic in song_player.c
- let input code choose a song with play_song_by_index(...)
That way, keyboard support only needs to decide when to play, not how to play sound.
Short summary
- Songs are stored as note arrays
- PIT provides timing
- PIT channel 2 + port 0x61 drive the PC speaker
- kernel now plays a default song during boot
- the code works
- Windows confirmed audio output works
- Arch issue was caused by container audio passthrough, not the playback logic itself
- for sound testing on Arch, run QEMU on the host