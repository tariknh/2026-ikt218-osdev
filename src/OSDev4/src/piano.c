#include "piano.h"
#include "irq.h"
#include "io.h"
#include "song/frequencies.h"

// ── VGA ───────────────────────────────────────────────────────────────────────
#define VGA_BUFFER  ((volatile uint16_t*)0xB8000)
#define VGA_WIDTH   80
#define VGA_HEIGHT  25

// ── Hardware ports ────────────────────────────────────────────────────────────
#define PIT_BASE_FREQ    1193182
#define PIT_CMD_PORT     0x43
#define PIT_CH2_PORT     0x42
#define PC_SPEAKER_PORT  0x61

// ── Colors ────────────────────────────────────────────────────────────────────
#define COL_NORMAL   0x07
#define COL_BRIGHT   0x0F
#define COL_PRESSED  0x70
#define COL_STATUS   0x0A
#define COL_TITLE    0x0B

// ── Layout ────────────────────────────────────────────────────────────────────
// 8 keys, each cell is 7 chars: "| XX   " plus final "|"
// Total width: 8*7 + 1 = 57 chars, starting at column 11
#define PIANO_START_COL  11
#define CELL_WIDTH       7

#define ROW_TITLE        2
#define ROW_HINT         3
#define ROW_BORDER_TOP   5
#define ROW_NOTES        6
#define ROW_KEYS         7
#define ROW_BORDER_BOT   8
#define ROW_STATUS       10

// ── Scan codes ────────────────────────────────────────────────────────────────
#define SC_A  0x1E
#define SC_S  0x1F
#define SC_D  0x20
#define SC_F  0x21
#define SC_G  0x22
#define SC_H  0x23
#define SC_J  0x24
#define SC_K  0x25

typedef struct {
    uint8_t     scancode;
    uint32_t    frequency;
    char        note[3];
    char        key;
} PianoKey;

static PianoKey keys[8] = {
    {SC_A, C4, "C4", 'A'},
    {SC_S, D4, "D4", 'S'},
    {SC_D, E4, "E4", 'D'},
    {SC_F, F4, "F4", 'F'},
    {SC_G, G4, "G4", 'G'},
    {SC_H, A4, "A4", 'H'},
    {SC_J, B4, "B4", 'J'},
    {SC_K, C5, "C5", 'K'},
};

static int currently_pressed = -1;

// ── Sound ─────────────────────────────────────────────────────────────────────

static void piano_play(uint32_t freq)
{
    uint32_t div = PIT_BASE_FREQ / freq;
    outb(PIT_CMD_PORT, 0xB6);
    outb(PIT_CH2_PORT, (uint8_t)(div & 0xFF));
    outb(PIT_CH2_PORT, (uint8_t)((div >> 8) & 0xFF));
    outb(PC_SPEAKER_PORT, inb(PC_SPEAKER_PORT) | 0x03);
}

static void piano_stop(void)
{
    outb(PC_SPEAKER_PORT, inb(PC_SPEAKER_PORT) & ~0x02);
}

// ── VGA helpers ───────────────────────────────────────────────────────────────

static void vga_put(int row, int col, char c, uint8_t color)
{
    VGA_BUFFER[row * VGA_WIDTH + col] = (uint16_t)(unsigned char)c | ((uint16_t)color << 8);
}

static void vga_puts(int row, int col, const char* s, uint8_t color)
{
    int i;
    for (i = 0; s[i]; i++)
        vga_put(row, col + i, s[i], color);
}

static void vga_clear(void)
{
    int i;
    for (i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++)
        VGA_BUFFER[i] = (uint16_t)' ' | ((uint16_t)COL_NORMAL << 8);
}

// ── Display ───────────────────────────────────────────────────────────────────

static void draw_key_cell(int idx, int pressed)
{
    int col = PIANO_START_COL + idx * CELL_WIDTH;
    uint8_t note_color = pressed ? COL_PRESSED : COL_BRIGHT;
    uint8_t key_color  = pressed ? COL_PRESSED : COL_NORMAL;

    // Note row: "| C4   "
    vga_put(ROW_NOTES, col,     '|', COL_BRIGHT);
    vga_put(ROW_NOTES, col + 1, ' ', note_color);
    vga_put(ROW_NOTES, col + 2, keys[idx].note[0], note_color);
    vga_put(ROW_NOTES, col + 3, keys[idx].note[1], note_color);
    vga_put(ROW_NOTES, col + 4, ' ', note_color);
    vga_put(ROW_NOTES, col + 5, ' ', note_color);

    // Key row: "| [A]  "
    vga_put(ROW_KEYS, col,     '|', COL_BRIGHT);
    vga_put(ROW_KEYS, col + 1, '[', key_color);
    vga_put(ROW_KEYS, col + 2, keys[idx].key, note_color);
    vga_put(ROW_KEYS, col + 3, ']', key_color);
    vga_put(ROW_KEYS, col + 4, ' ', key_color);
    vga_put(ROW_KEYS, col + 5, ' ', key_color);
}

static void update_status(int idx)
{
    int i;
    for (i = 0; i < 40; i++)
        vga_put(ROW_STATUS, PIANO_START_COL + i, ' ', COL_STATUS);

    if (idx < 0) {
        vga_puts(ROW_STATUS, PIANO_START_COL, "Now playing: ---", COL_STATUS);
    } else {
        vga_puts(ROW_STATUS, PIANO_START_COL,      "Now playing: ", COL_STATUS);
        vga_puts(ROW_STATUS, PIANO_START_COL + 13, keys[idx].note, COL_STATUS);
    }
}

static void draw_piano(void)
{
    int i;

    vga_puts(ROW_TITLE, 27, "== Piano Keyboard ==", COL_TITLE);
    vga_puts(ROW_HINT,  22, "Press A S D F G H J K to play", COL_NORMAL);

    for (i = 0; i < 8; i++) {
        int col = PIANO_START_COL + i * CELL_WIDTH;

        // Top border: "+------"
        vga_put(ROW_BORDER_TOP, col, '+', COL_BRIGHT);
        int j;
        for (j = 1; j < CELL_WIDTH; j++)
            vga_put(ROW_BORDER_TOP, col + j, '-', COL_BRIGHT);

        draw_key_cell(i, 0);

        // Bottom border: "+------"
        vga_put(ROW_BORDER_BOT, col, '+', COL_BRIGHT);
        for (j = 1; j < CELL_WIDTH; j++)
            vga_put(ROW_BORDER_BOT, col + j, '-', COL_BRIGHT);
    }

    // Closing border column
    int end_col = PIANO_START_COL + 8 * CELL_WIDTH;
    vga_put(ROW_BORDER_TOP, end_col, '+', COL_BRIGHT);
    vga_put(ROW_NOTES,      end_col, '|', COL_BRIGHT);
    vga_put(ROW_KEYS,       end_col, '|', COL_BRIGHT);
    vga_put(ROW_BORDER_BOT, end_col, '+', COL_BRIGHT);

    update_status(-1);
}

// ── Keyboard handler ──────────────────────────────────────────────────────────

static void piano_keyboard_handler(registers_t* regs)
{
    (void)regs;
    uint8_t scancode = inb(0x60);
    uint8_t released = scancode & 0x80;
    uint8_t base_sc  = scancode & 0x7F;

    int i;
    for (i = 0; i < 8; i++) {
        if (base_sc != keys[i].scancode) continue;

        if (!released) {
            if (currently_pressed >= 0)
                draw_key_cell(currently_pressed, 0);
            currently_pressed = i;
            draw_key_cell(i, 1);
            update_status(i);
            piano_play(keys[i].frequency);
        } else {
            if (currently_pressed == i) {
                draw_key_cell(i, 0);
                currently_pressed = -1;
                update_status(-1);
                piano_stop();
            }
        }
        return;
    }
}

// ── Init ──────────────────────────────────────────────────────────────────────

void piano_init(void)
{
    vga_clear();
    draw_piano();
    irq_register_handler(1, piano_keyboard_handler);
}
