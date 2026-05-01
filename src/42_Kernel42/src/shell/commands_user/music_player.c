#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <shell/music_player/songs.h>

#define SYS_ioctl 16
#define SYS_nanosleep 35

static inline uint32_t syscall(uint32_t num, uint32_t a, uint32_t b, uint32_t c) {
    uint32_t ret;
    __asm__ volatile(
        "movl %1, %%eax\n"
        "movl %2, %%ebx\n"
        "movl %3, %%ecx\n"
        "movl %4, %%edx\n"
        "int $0x80\n"
        "movl %%eax, %0"
        : "=g"(ret)
        : "g"(num), "g"(a), "g"(b), "g"(c)
        : "eax", "ebx", "ecx", "edx"
    );
    return ret;
}

static inline void pcspk_play(uint16_t freq) {
    syscall(SYS_ioctl, 0, 1, freq);
}

static inline void pcspk_stop(void) {
    syscall(SYS_ioctl, 0, 1, 0);
}

static inline void sleep_ms(uint32_t ms) {
    syscall(SYS_nanosleep, ms * 1000, 0, 0);
}

static void play_song(const song_t* song) {
    for (size_t i = 0; i < song->length; i++) {
        if (song->notes[i].freq > 0) {
            pcspk_play(song->notes[i].freq);
            sleep_ms(song->notes[i].duration);
            pcspk_stop();
        } else {
            sleep_ms(song->notes[i].duration);
        }
        sleep_ms(50);
    }
}

int cmd_music_player_user(int argc, char** argv) {
    if (argc < 2 || strcmp(argv[1], "list") == 0) {
        printf("Available songs:\n");
        for (size_t i = 0; i < NUM_SONGS; i++) {
            printf("  %s\n", songs[i].name);
        }
        printf("Usage: music_player <song_name>\n");
        return 0;
    }

    const char* song_name = argv[1];
    for (size_t i = 0; i < NUM_SONGS; i++) {
        if (strcmp(song_name, songs[i].name) == 0) {
            printf("Playing: %s\n", songs[i].name);
            play_song(&songs[i]);
            printf("Done.\n");
            return 0;
        }
    }

    printf("Unknown song: %s\n", song_name);
    printf("Type 'music_player' to see available songs.\n");
    return 0;
}