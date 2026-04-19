#include <starfield.h>
#include <input.h>
#include <pit.h>
#include <libc/stdint.h>
#include <memory.h>

/*
 * Starfield Simulator
 *
 * Each star has a 3D position (x, y, z).  Every frame we decrease z,
 * project the star onto the 2D screen using perspective division,
 * and pick a character based on how "close" the star is.
 * When a star flies off-screen or z reaches 0, it is respawned
 * at a random position far away.
 *
 * All rendering goes directly to the VGA text buffer at 0xB8000
 * for flicker-free drawing.
 */

#define VGA_ADDR   0xB8000
#define COLS       80
#define ROWS       25
#define NUM_STARS  80
#define MAX_DEPTH  64
#define FRAME_MS   50      /* ~20 FPS */

/* Star data */
typedef struct {
    int32_t x;      /* -1000 .. 1000 */
    int32_t y;      /* -1000 .. 1000 */
    int32_t z;      /* 1 .. MAX_DEPTH */
} Star;

static Star stars[NUM_STARS];

/* ── Simple PRNG ── */
static uint32_t sf_rng = 0;

static uint32_t sf_rand(void)
{
    sf_rng ^= sf_rng << 13;
    sf_rng ^= sf_rng >> 17;
    sf_rng ^= sf_rng << 5;
    return sf_rng;
}

static int32_t rand_range(int32_t lo, int32_t hi)
{
    uint32_t range = (uint32_t)(hi - lo + 1);
    return lo + (int32_t)(sf_rand() % range);
}

/* ── Initialise / respawn a star ── */
static void spawn_star(Star *s)
{
    s->x = rand_range(-1000, 1000);
    s->y = rand_range(-500, 500);
    s->z = rand_range(MAX_DEPTH / 2, MAX_DEPTH);
}

/* ── Pick character and colour based on depth ── */
static void star_appearance(int32_t z, char *ch, uint8_t *color)
{
    if (z < MAX_DEPTH / 6) {
        *ch = '@';
        *color = 0x0F;   /* bright white – very close */
    } else if (z < MAX_DEPTH / 3) {
        *ch = 'O';
        *color = 0x0F;   /* bright white */
    } else if (z < MAX_DEPTH / 2) {
        *ch = 'o';
        *color = 0x07;   /* light grey */
    } else if (z < (MAX_DEPTH * 3) / 4) {
        *ch = '*';
        *color = 0x08;   /* dark grey */
    } else {
        *ch = '.';
        *color = 0x08;   /* dark grey – far away */
    }
}

/* ── Direct VGA access ── */
static volatile uint16_t *vga = (volatile uint16_t *)VGA_ADDR;

static void vga_clear(void)
{
    for (int i = 0; i < COLS * ROWS; i++)
        vga[i] = (uint16_t)(' ' | (0x00 << 8));  /* black on black */
}

static void vga_put(int row, int col, char c, uint8_t color)
{
    if (row >= 0 && row < ROWS && col >= 0 && col < COLS)
        vga[row * COLS + col] = (uint16_t)((uint8_t)c | ((uint16_t)color << 8));
}

/* ── Main starfield loop ── */
void starfield_run(void)
{
    sf_rng = get_current_tick() ^ 0xCAFEBABE;

    /* Initialise all stars */
    for (int i = 0; i < NUM_STARS; i++)
        spawn_star(&stars[i]);

    /* We keep a shadow buffer of previous star screen positions
     * so we can erase them without clearing the whole screen */
    int16_t prev_sx[NUM_STARS];
    int16_t prev_sy[NUM_STARS];
    for (int i = 0; i < NUM_STARS; i++) {
        prev_sx[i] = -1;
        prev_sy[i] = -1;
    }

    vga_clear();

    /* Show title bar */
    const char *title = " STARFIELD - Press ESC to exit ";
    int tlen = 0;
    while (title[tlen]) tlen++;
    int tstart = (COLS - tlen) / 2;
    for (int i = 0; i < tlen; i++)
        vga_put(0, tstart + i, title[i], 0x70);  /* black on grey */

    int center_x = COLS / 2;
    int center_y = ROWS / 2;

    while (1) {
        /* Check for ESC */
        char c = getchar_nonblock();
        if (c == 0x1B)
            break;

        for (int i = 0; i < NUM_STARS; i++) {
            /* Erase old position */
            if (prev_sx[i] >= 0 && prev_sy[i] >= 0)
                vga_put(prev_sy[i], prev_sx[i], ' ', 0x00);

            /* Move star closer */
            stars[i].z -= 1;

            /* Respawn if too close or off-screen */
            if (stars[i].z <= 0) {
                spawn_star(&stars[i]);
                prev_sx[i] = -1;
                prev_sy[i] = -1;
                continue;
            }

            /* Perspective projection: screen_x = x / z, screen_y = y / z */
            int32_t sx = center_x + (stars[i].x * 2) / stars[i].z;
            int32_t sy = center_y + (stars[i].y) / stars[i].z;

            /* Off-screen? Respawn */
            if (sx < 0 || sx >= COLS || sy < 1 || sy >= ROWS) {
                spawn_star(&stars[i]);
                prev_sx[i] = -1;
                prev_sy[i] = -1;
                continue;
            }

            /* Draw star */
            char ch;
            uint8_t color;
            star_appearance(stars[i].z, &ch, &color);
            vga_put((int)sy, (int)sx, ch, color);

            prev_sx[i] = (int16_t)sx;
            prev_sy[i] = (int16_t)sy;
        }

        sleep_interrupt(FRAME_MS);
    }
}
