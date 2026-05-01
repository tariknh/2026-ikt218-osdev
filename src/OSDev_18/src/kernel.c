#include <libc/stdint.h>
#include <kernel/terminal.h>
#include <kernel/gdt.h>
#include <kernel/idt.h>
#include <kernel/interrupt.h>
#include <kernel/keyboard.h>
#include <kernel/memory.h>
#include <kernel/pit.h>
#include <songApp/song.h>
#include <songApp/frequencies.h>
#include <snakeApp/snake.h>
#include <pianoApp/piano.h>

extern uint32_t end;

void PlayMusic(void) {
    Song songs[] = {
        {music_1, sizeof(music_1) / sizeof(Note)},
        {music_2, sizeof(music_2) / sizeof(Note)},
        {music_3, sizeof(music_3) / sizeof(Note)},
        {music_4, sizeof(music_4) / sizeof(Note)},
        {music_5, sizeof(music_5) / sizeof(Note)},
        {music_6, sizeof(music_6) / sizeof(Note)}
    };

    uint32_t songCount = sizeof(songs) / sizeof(Song);

    SongPlayer* player = CreateSongPlayer();

    if (!player) {
        TerminalWriteString("Failed to create SongPlayer.\n");
        return;
    }

    for(uint32_t i = 0; i < songCount; i++) {
        player->play_song(&songs[i]);
        SleepInterrupt(1000);
    }
}

void main(void) {
    TerminalInitialize();

    TerminalWriteString("Hello World (TerminalWriteString)\n");

    // Bring up the low-level pieces in dependency order: CPU segments first,
    // then interrupt dispatch and the timer, then input, heap setup, and
    // finally paging once the kernel has the memory structures it needs.
    GdtInitialize();
    IdtInitialize();
    PitInitialize();

    RegisterInterruptHandler(IRQ1, KeyboardHandler);

    InitKernelMemory(&end);
    InitPaging();
    PrintMemoryLayout();

    while (1) {
        TerminalWriteString("Enter application number:\n");
        TerminalWriteString("0. Play Music\n");
        TerminalWriteString("1. Play Snake\n");
        TerminalWriteString("2. Play Piano\n");
        char input = TerminalGetChar();

        switch (input) {
            case '0':
                TerminalClear();
                PlayMusic();
                TerminalClear();
                break;
            case '1':
                TerminalClear();
                PlayGame();
                TerminalClear();
                break;
            case '2':
                TerminalClear();
                PlayPiano();
                TerminalClear();
                break;
            default:
                TerminalWriteString("\nInvalid application number.\n");
                SleepInterrupt(1000);
                TerminalClear();
                break;
        }
    }

    for (;;) {
        __asm__ volatile("hlt");
    }
}
