#include <menu.h>
#include <input.h>
#include <shell.h>
#include <typingtest.h>
#include <starfield.h>
#include <song.h>
#include <pit.h>
#include <pic.h>
#include <libc/stdint.h>
#include <libc/stdio.h>

extern void terminal_clear(void);

/* ── Shutdown (same logic as shell.c) ── */
static void menu_shutdown(void)
{
    terminal_clear();
    printf("Shutting down");
    for (int i = 3; i > 0; i--) {
        printf(" %d...", i);
        sleep_interrupt(1000);
    }
    printf("\nGoodbye!\n");
    sleep_interrupt(500);

    outb(0xF4, 0x00);
    __asm__ volatile("outw %%ax, %%dx" : : "a"(0x2000), "d"(0x604));
    __asm__ volatile("cli");
    while (1) __asm__ volatile("hlt");
}

/* ── Draw the menu screen ── */
static void draw_menu(void)
{
    terminal_clear();
    printf("==========================================\n");
    printf("     Welcome to lazarOS!\n");
    printf("==========================================\n\n");
    printf("  1. Interactive Shell\n");
    printf("  2. Typing Speed Test\n");
    printf("  3. Starfield Simulator\n");
    printf("  4. Play Song (Super Mario)\n");
    printf("  5. Shutdown\n");
    printf("\n");
    printf("  Select an option (1-5): ");
}

/* ── Play song without infinite loop ── */
static void menu_play_song(void)
{
    terminal_clear();
    printf("Playing Super Mario theme...\n");
    printf("(Press ESC after the song to return to menu)\n\n");

    Song song = { music_1, (uint32_t)(sizeof(music_1) / sizeof(music_1[0])) };
    play_song_impl(&song);

    printf("\nSong finished! Press any key...\n");
    getchar();
}

/* ── Main menu loop ── */
void menu_run(void)
{
    while (1) {
        draw_menu();

        char choice = getchar();
        printf("%c\n", choice);

        switch (choice) {
        case '1':
            shell_run();
            break;
        case '2':
            typingtest_run();
            break;
        case '3':
            starfield_run();
            break;
        case '4':
            menu_play_song();
            break;
        case '5':
            menu_shutdown();
            break;
        default:
            printf("Invalid choice. Press 1-5.\n");
            sleep_interrupt(1000);
            break;
        }
    }
}
