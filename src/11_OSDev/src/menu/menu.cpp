extern "C" {
    #include "menu/menu.h"
    #include "terminal.h"
    #include "kernel/pit.h"
    #include "kernel/memory.h"
    #include "isr.h"
    #include "sysmon/sysmon.h"
    #include "song/song.h"
    #include "libc/stdio.h"
}


static void draw_header(const char *title, vga_color color)
{
    terminal_clear();
    terminal_setpos(0, 0);
    terminal_setcolor(VGA_COLOR_WHITE, color);
    for (uint32_t i = 0; i < VGA_WIDTH; i++) terminal_putchar(' ');
    terminal_setpos(0, (VGA_WIDTH - 60) / 2);
    terminal_write(title);
    terminal_setpos(1, 0);
    terminal_setcolor(VGA_COLOR_DARK_GREY, VGA_COLOR_BLACK);
    for (uint32_t i = 0; i < VGA_WIDTH - 1; i++) terminal_putchar('=');
    terminal_setpos(2, 0);
}

static void draw_footer(void)
{
    terminal_setpos(23, 0);
    terminal_setcolor(VGA_COLOR_DARK_GREY, VGA_COLOR_BLACK);
    for (uint32_t i = 0; i < VGA_WIDTH - 1; i++) terminal_putchar('-');
    terminal_setpos(24, 0);
    terminal_setcolor(VGA_COLOR_DARK_GREY, VGA_COLOR_BLACK);
    terminal_write("  Press 'q' to return to the main menu.");
}

/* 
 * Main Menu
 */

static void draw_menu(void)
{
    terminal_clear();

    /* Title bar */
    terminal_setpos(0, 0);
    terminal_setcolor(VGA_COLOR_WHITE, VGA_COLOR_BLUE);
    for (uint32_t i = 0; i < VGA_WIDTH; i++) terminal_putchar(' ');
    terminal_setpos(0, 24);
    terminal_write("UiAOS - OS Exam Demo Menu");

    terminal_setpos(1, 0);
    terminal_setcolor(VGA_COLOR_DARK_GREY, VGA_COLOR_BLACK);
    for (uint32_t i = 0; i < VGA_WIDTH - 1; i++) terminal_putchar('=');

    /* Demo list */
    terminal_setpos(3, 4);
    terminal_setcolor(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    terminal_write("[2]");
    terminal_setcolor(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    terminal_write("  Assignment 2 - GDT & Text Mode Terminal");

    terminal_setpos(5, 4);
    terminal_setcolor(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    terminal_write("[3]");
    terminal_setcolor(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    terminal_write("  Assignment 3 - Interrupts, IDT, ISR & Keyboard");

    terminal_setpos(7, 4);
    terminal_setcolor(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    terminal_write("[4]");
    terminal_setcolor(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    terminal_write("  Assignment 4 - Memory Management & PIT");

    terminal_setpos(9, 4);
    terminal_setcolor(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    terminal_write("[5]");
    terminal_setcolor(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    terminal_write("  Assignment 5 - PC Speaker Music Player");

    terminal_setpos(11, 4);
    terminal_setcolor(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    terminal_write("[6]");
    terminal_setcolor(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    terminal_write("  Assignment 6 - System Monitor (Improvisation)");

    terminal_setpos(14, 0);
    terminal_setcolor(VGA_COLOR_DARK_GREY, VGA_COLOR_BLACK);
    for (uint32_t i = 0; i < VGA_WIDTH - 1; i++) terminal_putchar('-');
    terminal_setpos(15, 4);
    terminal_setcolor(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    terminal_write("Press the number key for the demo you want to run.");
}

/*
 * Assignment 2 - GDT & Text Mode Terminal
  */

static void demo_gdt_terminal(void)
{
    draw_header(
        "Assignment 2 - GDT & Text Mode Terminal                    ",
        VGA_COLOR_GREEN);

    terminal_setcolor(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    terminal_write("\n  Hello World!\n\n");

    terminal_setcolor(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    terminal_write("  GDT Descriptors (flat model, base=0, limit=4 GB):\n");
    terminal_setcolor(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    terminal_write("    Slot 0  NULL descriptor  (required by CPU)\n");
    terminal_write("    Slot 1  Code segment     selector=0x08  access=0x9A  gran=0xCF\n");
    terminal_write("    Slot 2  Data segment     selector=0x10  access=0x92  gran=0xCF\n\n");

    terminal_setcolor(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    terminal_write("  GDT loaded via lgdt, segment registers reloaded via far jump.\n\n");

    terminal_setcolor(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    terminal_write("  printf format specifiers:\n");
    terminal_setcolor(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    printf("    %%d  ->  %d\n",   -42);
    printf("    %%u  ->  %u\n",   42u);
    printf("    %%x  ->  0x%x\n", 255u);
    printf("    %%X  ->  0x%X\n", 255u);
    printf("    %%s  ->  %s\n",   "kernel string");
    printf("    %%c  ->  %c\n",   'K');

    draw_footer();
    kb_flush();
    while (kb_getchar() != 'q') {}
}

/* 
 * Assignment 3 - Interrupts, IDT, ISR & Keyboard
 */

static void demo_interrupts(void)
{
    draw_header(
        "Assignment 3 - Interrupts, IDT, ISR & Keyboard             ",
        VGA_COLOR_MAGENTA);

    terminal_setcolor(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    terminal_write("\n  Interrupt Descriptor Table:\n");
    terminal_setcolor(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    terminal_write("    Vectors  0-31  CPU exception ISRs  (32 handlers)\n");
    terminal_write("    Vectors 32-39  Master PIC IRQs     (IRQ 0-7)\n");
    terminal_write("    Vectors 40-47  Slave  PIC IRQs     (IRQ 8-15)\n\n");

    terminal_setcolor(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    terminal_write("  PIC remapped so hardware IRQs start at vector 32.\n\n");

    terminal_setcolor(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    terminal_write("  IRQ counters since boot:\n");
    terminal_setcolor(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    printf("    IRQ 0  PIT Timer    : %u ticks\n",       get_irq_count(0));
    printf("    IRQ 1  PS/2 Keyboard: %u keypresses\n",  get_irq_count(1));

    terminal_setpos(15, 0);
    terminal_setcolor(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    terminal_write("  Keyboard logger active - type below:\n  > ");
    terminal_setcolor(VGA_COLOR_WHITE, VGA_COLOR_BLACK);

    
    kb_flush();
    while (1) {
        char c = kb_getchar();
        if (c == 'q' || c == 'Q') break;
    }
}

/* 
 * Assignment 4 - Memory Management & PIT
 */

static void demo_memory_pit(void)
{
    draw_header(
        "Assignment 4 - Memory Management & PIT                     ",
        VGA_COLOR_CYAN);

    terminal_setcolor(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    terminal_write("\n  Heap layout:\n");
    terminal_setcolor(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    printf("    Start  : 0x%x\n",  get_heap_begin());
    printf("    End    : 0x%x\n",  get_heap_end());
    printf("    Total  : %u bytes\n", get_heap_total());
    printf("    Used   : %u bytes\n\n", get_memory_used());

    terminal_setcolor(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    terminal_write("  malloc / free test:\n");
    terminal_setcolor(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    void *p1 = malloc(256);
    void *p2 = malloc(512);
    printf("    malloc(256) -> 0x%x\n", (uint32_t)p1);
    printf("    malloc(512) -> 0x%x\n", (uint32_t)p2);
    free(p1);
    free(p2);
    terminal_write("    free(p1), free(p2) -> OK\n\n");

    terminal_setcolor(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    terminal_write("  PIT sleep demo  (5 iterations, press 'q' to skip):\n");
    terminal_setcolor(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);

    kb_flush();
    for (uint32_t i = 0; i < 5; i++) {
        if (kb_peek() == 'q' || kb_peek() == 'Q') { kb_consume(); break; }
        printf("    [%u] sleep_busy(500) ...", i);
        sleep_busy(500);
        printf(" done.  sleep_interrupt(500) ...");
        sleep_interrupt(500);
        printf(" done.\n");
    }

    draw_footer();
    kb_flush();
    while (kb_getchar() != 'q') {}
}

/* 
 * Assignment 5 - Music Player
 */

static void demo_music(SongPlayer *player)
{
    draw_header(
        "Assignment 5 - PC Speaker Music Player                     ",
        VGA_COLOR_BROWN);

    terminal_setcolor(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    terminal_write("\n  Song: Tetris Theme (Korobeiniki) - 40 notes at 150 BPM\n");
    terminal_setcolor(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    terminal_write("  Each note's frequency and duration are printed as it plays.\n");
    terminal_write("  Press 'q' to stop (takes effect between songs).\n\n");

    Song songs[] = { {music_1, music_1_count} };
    uint32_t n_songs = sizeof(songs) / sizeof(Song);

    kb_flush();
    while (kb_peek() != 'q' && kb_peek() != 'Q') {
        for (uint32_t i = 0; i < n_songs; i++) {
            terminal_setcolor(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
            printf("  >> Playing song %u...\n", i + 1);
            terminal_setcolor(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
            player->play_song(&songs[i]);
            terminal_setcolor(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
            printf("  >> Finished.\n\n");
            if (kb_peek() == 'q' || kb_peek() == 'Q') break;
        }
    }
    kb_consume();
}

/* 
 * Assignment 6 - System Monitor
 */

static void demo_sysmon(void)
{
    /* sysmon_run() draws its own full-screen dashboard and returns
       when the user presses 'q'. */
    sysmon_run();
}

/* 
 * Menu entry point
 */

extern "C" void menu_run(SongPlayer *player)
{
    while (1) {
        draw_menu();
        kb_flush();
        char choice = kb_getchar();
        switch (choice) {
            case '2': demo_gdt_terminal(); break;
            case '3': demo_interrupts();   break;
            case '4': demo_memory_pit();   break;
            case '5': demo_music(player);  break;
            case '6': demo_sysmon();       break;
            default:  break;   /* any other key just redraws the menu */
        }
    }
}
