extern "C" {
    #include "sysmon/sysmon.h"
    #include "terminal.h"
    #include "kernel/pit.h"
    #include "kernel/memory.h"
    #include "isr.h"
    #include "libc/stdio.h"
}

#define BAR_WIDTH 24

/*  Helpers  */


static void draw_separator(uint32_t row)
{
    terminal_setpos(row, 0);
    terminal_setcolor(VGA_COLOR_DARK_GREY, VGA_COLOR_BLACK);
    for (uint32_t i = 0; i < VGA_WIDTH - 1; i++)
        terminal_putchar('-');
}


static void draw_title(uint32_t tick)
{
    static const char spinner[] = {'|', '/', '-', '\\'};
    const char *title = "UiAOS System Monitor";
    const uint32_t title_len = 20;

    terminal_setpos(0, 0);
    terminal_setcolor(VGA_COLOR_WHITE, VGA_COLOR_BLUE);

    for (uint32_t i = 0; i < VGA_WIDTH; i++)
        terminal_putchar(' ');

    terminal_setpos(0, (VGA_WIDTH - title_len) / 2);
    terminal_write(title);

    terminal_setpos(0, VGA_WIDTH - 2);
    terminal_putchar(spinner[(tick / 1000) % 4]);
}


static void draw_section(uint32_t row, const char *name)
{
    terminal_setpos(row, 0);
    terminal_setcolor(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    printf("  %s", name);
}


static void draw_label(const char *text)
{
    terminal_setcolor(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    terminal_write(text);
}


static void draw_uptime(uint32_t row, uint32_t ticks)
{
    uint32_t total_secs = ticks / 1000;
    uint32_t hours      = total_secs / 3600;
    uint32_t mins       = (total_secs % 3600) / 60;
    uint32_t secs       = total_secs % 60;

    terminal_setpos(row, 0);
    draw_label("    Uptime    ");
    terminal_setcolor(VGA_COLOR_WHITE, VGA_COLOR_BLACK);

    if (hours < 10) terminal_putchar('0');
    printf("%u:", hours);
    if (mins < 10) terminal_putchar('0');
    printf("%u:", mins);
    if (secs < 10) terminal_putchar('0');
    printf("%u", secs);
}


static void draw_bar(uint32_t used, uint32_t total)
{
    uint32_t pct    = total ? (used * 100) / total : 0;
    uint32_t filled = (pct * BAR_WIDTH) / 100;

    terminal_setcolor(VGA_COLOR_DARK_GREY, VGA_COLOR_BLACK);
    terminal_putchar('[');

    for (uint32_t i = 0; i < BAR_WIDTH; i++) {
        if (i < filled) {
            terminal_setcolor(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
            terminal_putchar('#');
        } else {
            terminal_setcolor(VGA_COLOR_DARK_GREY, VGA_COLOR_BLACK);
            terminal_putchar('.');
        }
    }

    terminal_setcolor(VGA_COLOR_DARK_GREY, VGA_COLOR_BLACK);
    terminal_putchar(']');
    terminal_putchar(' ');
    terminal_setcolor(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    printf("%u%%", pct);
}

/*  Main draw routine  */

static void draw_screen(uint32_t tick)
{
    uint32_t used  = get_memory_used();
    uint32_t total = get_heap_total();
    uint32_t hbegin = get_heap_begin();
    uint32_t hend   = get_heap_end();

    terminal_clear();

    /*  Row 0: Title bar  */
    draw_title(tick);

    /*  Row 1: Separator  */
    draw_separator(1);

    /*  Row 2: Uptime  */
    draw_uptime(2, tick);

    /*  Row 3: Separator  */
    draw_separator(3);

    /*  Row 4: Memory section header  */
    draw_section(4, "Memory");

    /*  Row 5: Heap address range  */
    terminal_setpos(5, 0);
    draw_label("    Heap      ");
    terminal_setcolor(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    printf("0x%x  ->  0x%x", hbegin, hend);

    /*  Row 6: Memory used with progress bar  */
    terminal_setpos(6, 0);
    draw_label("    Used      ");
    terminal_setcolor(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    printf("%u bytes  ", used);
    draw_bar(used, total);

    /*  Row 7: Memory free  */
    terminal_setpos(7, 0);
    draw_label("    Free      ");
    terminal_setcolor(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    printf("%u bytes", total > used ? total - used : 0);

    /*  Row 8: Total heap size  */
    terminal_setpos(8, 0);
    draw_label("    Total     ");
    terminal_setcolor(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    printf("%u bytes", total);

    /*  Row 9: Separator  */
    draw_separator(9);

    /*  Row 10: Interrupts section header  */
    draw_section(10, "Interrupts");

    /*  Row 11: PIT IRQ counter  */
    terminal_setpos(11, 0);
    draw_label("    IRQ 0   PIT Timer     ");
    terminal_setcolor(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    printf("%u ticks", get_irq_count(0));

    /*  Row 12: Keyboard IRQ counter  */
    terminal_setpos(12, 0);
    draw_label("    IRQ 1   Keyboard      ");
    terminal_setcolor(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    printf("%u keypresses", get_irq_count(1));

    /*  Row 13: Separator  */
    draw_separator(13);

    /*  Row 14: Keyboard section header  */
    draw_section(14, "Keyboard");

    /*  Row 15: Last key pressed  */
    terminal_setpos(15, 0);
    draw_label("    Last key  ");
    terminal_setcolor(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    char last = get_last_keypress();
    if (last >= ' ' && last <= '~')
        printf("'%c'", last);
    else
        terminal_write("---");

    /*  Row 16: Separator  */
    draw_separator(16);

    /*  Row 17: Status line  */
    terminal_setpos(17, 0);
    terminal_setcolor(VGA_COLOR_DARK_GREY, VGA_COLOR_BLACK);
    terminal_write("  Refreshing every second");

    /*  Row 18: Exit hint  */
    terminal_setpos(18, 0);
    terminal_setcolor(VGA_COLOR_DARK_GREY, VGA_COLOR_BLACK);
    terminal_write("  Press 'q' to return to the main menu.");
}

/*  Entry point  */

extern "C" void sysmon_run(void)
{
    kb_flush();
    while (1) {
        draw_screen(get_current_tick());

        /* Check for 'q' every 100 ms so the exit feels instant.
           Ten 100 ms slices add up to the one-second refresh period. */
        for (uint32_t slice = 0; slice < 10; slice++) {
            sleep_interrupt(100);
            char k = kb_peek();
            if (k == 'q' || k == 'Q') {
                kb_consume();
                return;
            }
        }
    }
}
