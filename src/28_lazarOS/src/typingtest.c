#include <typingtest.h>
#include <input.h>
#include <pit.h>
#include <libc/stdint.h>
#include <libc/stdio.h>
#include <memory.h>

extern void terminal_clear(void);

/* ── Word bank for the typing challenges ── */
static const char *words[] = {
    "kernel",
    "interrupt",
    "memory",
    "paging",
    "assembly",
    "bootloader",
    "operating",
    "system",
    "hardware",
    "register",
    "segment",
    "descriptor",
    "virtual",
    "physical",
    "scheduler",
    "process",
    "stack",
    "heap",
    "pointer",
    "buffer"
};
#define WORD_COUNT 20
#define NUM_ROUNDS 5

/* ── Simple PRNG seeded from PIT ticks ── */
static uint32_t rng_state = 0;

static uint32_t rng_next(void)
{
    rng_state ^= rng_state << 13;
    rng_state ^= rng_state >> 17;
    rng_state ^= rng_state << 5;
    return rng_state;
}

static int str_length(const char *s)
{
    int n = 0;
    while (s[n]) n++;
    return n;
}

/* ── VGA direct write for coloured text ── */
#define VGA_ADDR  0xB8000
#define VGA_COLS  80

static void vga_put_colored(int row, int col, char c, uint8_t color)
{
    volatile unsigned short *vga = (volatile unsigned short *)VGA_ADDR;
    vga[row * VGA_COLS + col] = (unsigned short)((unsigned char)c | (color << 8));
}

/* ── Show the target word with colour feedback ── */
static void draw_word(int row, const char *word, const char *typed, int typed_len)
{
    int word_len = str_length(word);
    int col_start = 4;   /* indent a bit */

    for (int i = 0; i < word_len; i++) {
        uint8_t color;
        if (i < typed_len) {
            if (typed[i] == word[i])
                color = 0x0A;  /* green – correct */
            else
                color = 0x0C;  /* red – wrong */
        } else {
            color = 0x07;      /* grey – not yet typed */
        }
        vga_put_colored(row, col_start + i, word[i], color);
    }

    /* Clear any leftover characters beyond the word */
    for (int i = word_len; i < 40; i++)
        vga_put_colored(row, col_start + i, ' ', 0x07);
}

/* ── Run a single round; returns elapsed milliseconds ── */
static uint32_t run_round(int round_num)
{
    /* Pick a random word */
    rng_state ^= get_current_tick();
    uint32_t idx = rng_next() % WORD_COUNT;
    const char *word = words[idx];
    int word_len = str_length(word);

    terminal_clear();
    printf("=== Typing Speed Test - Round %d / %d ===\n\n", round_num, NUM_ROUNDS);
    printf("Type the word below as fast as you can:\n\n");

    /* Show the word in grey initially */
    draw_word(4, word, "", 0);

    printf("\n\n> ");

    /* Wait for the first keystroke to start the timer */
    char typed[64];
    int pos = 0;

    char first = getchar();
    uint32_t start_tick = get_current_tick();

    /* Process first character */
    if (first >= ' ' && first <= '~') {
        typed[pos++] = first;
        printf("%c", first);
        draw_word(4, word, typed, pos);
    }

    /* Read remaining characters */
    while (pos < word_len) {
        char c = getchar();

        if (c == '\b' || c == 0x7F) {
            if (pos > 0) {
                pos--;
                printf("\b \b");
                draw_word(4, word, typed, pos);
            }
            continue;
        }

        if (c == 0x1B) {   /* ESC – abort */
            return 0;
        }

        if (c >= ' ' && c <= '~') {
            typed[pos++] = c;
            printf("%c", c);
            draw_word(4, word, typed, pos);
        }
    }

    uint32_t end_tick = get_current_tick();
    uint32_t elapsed_ms = end_tick - start_tick;

    typed[pos] = '\0';

    /* Check correctness */
    int correct = 1;
    for (int i = 0; i < word_len; i++) {
        if (typed[i] != word[i]) {
            correct = 0;
            break;
        }
    }

    printf("\n\n");
    if (correct) {
        printf("Correct! ");
    } else {
        printf("Not quite right. ");
    }
    printf("Time: %d ms\n", elapsed_ms);

    if (elapsed_ms > 0) {
        /* WPM = (characters / 5) / (time in minutes)
         * = (word_len / 5) / (elapsed_ms / 60000)
         * = (word_len * 60000) / (5 * elapsed_ms) */
        uint32_t wpm = (word_len * 60000) / (5 * elapsed_ms);
        printf("Speed: %d WPM\n", wpm);
    }

    printf("\nPress any key for the next round...");
    getchar();

    return elapsed_ms;
}

/* ── Main entry point ── */
void typingtest_run(void)
{
    rng_state = get_current_tick() ^ 0xDEADBEEF;

    terminal_clear();
    printf("======================================\n");
    printf("       TYPING SPEED TEST\n");
    printf("======================================\n\n");
    printf("You will be shown %d words, one at a time.\n", NUM_ROUNDS);
    printf("Type each word as fast and accurately as you can.\n");
    printf("The timer starts when you press the first key.\n\n");
    printf("Press any key to begin...\n");
    getchar();

    uint32_t total_ms = 0;
    int rounds_completed = 0;

    for (int i = 1; i <= NUM_ROUNDS; i++) {
        uint32_t ms = run_round(i);
        if (ms == 0) {   /* user pressed ESC */
            printf("\nTest aborted.\n");
            sleep_interrupt(1000);
            return;
        }
        total_ms += ms;
        rounds_completed++;
    }

    /* Show final results */
    terminal_clear();
    printf("======================================\n");
    printf("          RESULTS\n");
    printf("======================================\n\n");
    printf("Rounds completed:   %d\n", rounds_completed);
    printf("Total time:         %d ms\n", total_ms);

    if (rounds_completed > 0) {
        uint32_t avg_ms = total_ms / rounds_completed;
        printf("Average per word:   %d ms\n", avg_ms);
    }

    printf("\nPress any key to return to the menu...\n");
    getchar();
}
