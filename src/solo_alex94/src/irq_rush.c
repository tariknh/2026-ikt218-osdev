#include <libc/stddef.h>
#include <libc/stdint.h>
#include "kernel/irq_rush.h"
#include "kernel/pit.h"
#include "ports.h"

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_MEMORY 0xB8000

#define COLOR_WHITE 0x0F
#define COLOR_GRAY 0x08
#define COLOR_GREEN 0x0A
#define COLOR_CYAN 0x0B
#define COLOR_RED 0x0C
#define COLOR_MAGENTA 0x0D
#define COLOR_YELLOW 0x0E

#define PIT_SPEAKER_COMMAND 0xB6

#define MAX_EVENTS 16
#define FIRST_EVENT_ROW 5
#define HIT_ROW 20
#define FEEDBACK_ROW 21
#define MISS_ROW 22
#define HIT_WINDOW 2

#define EVENT_NORMAL 0
#define EVENT_PRIORITY 1

#define SCREEN_MENU 0
#define SCREEN_PLAYING 1
#define SCREEN_DIAGNOSTICS 2
#define SCREEN_SPEAKER_TEST 3
#define SCREEN_CREDITS 4
#define SCREEN_HELP 5
#define SCREEN_GAME_OVER 6

#define SC_ESCAPE 0x01
#define SC_1 0x02
#define SC_2 0x03
#define SC_3 0x04
#define SC_4 0x05
#define SC_5 0x06
#define SC_6 0x07
#define SC_Q 0x10
#define SC_R 0x13
#define SC_A 0x1E
#define SC_S 0x1F
#define SC_D 0x20
#define SC_F 0x21

#define BEEP_QUEUE_SIZE 16

typedef struct {
    uint8_t active;
    uint8_t lane;
    uint8_t row;
    uint8_t type;
    uint32_t spawn_tick;
} irq_event_t;

typedef struct {
    uint32_t frequency;
    uint32_t duration;
} beep_event_t;

static volatile uint16_t* const vga_buffer = (uint16_t*)VGA_MEMORY;

static irq_event_t events[MAX_EVENTS];

static uint8_t current_screen = SCREEN_MENU;
static uint8_t difficulty = 1;
static uint8_t needs_render = 1;

static uint32_t score = 0;
static uint32_t combo = 0;
static uint32_t best_combo = 0;
static uint32_t panic_level = 0;
static uint32_t events_serviced = 0;
static uint32_t events_missed = 0;

static uint32_t last_move_tick = 0;
static uint32_t last_spawn_tick = 0;
static uint32_t last_render_tick = 0;

static uint32_t move_delay = 160;
static uint32_t spawn_delay = 650;

static uint32_t rng_state = 0x1234ABCD;

static uint8_t last_scancode = 0;
static const char* last_action = "System ready.";

static uint8_t feedback_active[4] = {0};
static char feedback_symbol[4] = {' ', ' ', ' ', ' '};
static uint8_t feedback_color[4] = {COLOR_WHITE, COLOR_WHITE, COLOR_WHITE, COLOR_WHITE};
static uint32_t feedback_until_tick[4] = {0};

static uint32_t speaker_stop_tick = 0;
static uint32_t current_speaker_frequency = 0;

static beep_event_t beep_queue[BEEP_QUEUE_SIZE];
static uint8_t beep_head = 0;
static uint8_t beep_tail = 0;
static uint8_t beep_count = 0;

static const uint8_t lane_x[4] = {8, 24, 40, 56};
static const char* lane_names[4] = {"TIMER", "KEYBOARD", "MEMORY", "SOUND"};

static uint16_t make_vga_entry(char character, uint8_t color) {
    return (uint16_t)((uint8_t)character) | ((uint16_t)color << 8);
}

static void screen_put_char(uint32_t x, uint32_t y, char character, uint8_t color) {
    if (x >= VGA_WIDTH || y >= VGA_HEIGHT) {
        return;
    }

    vga_buffer[y * VGA_WIDTH + x] = make_vga_entry(character, color);
}

static void screen_put_string(uint32_t x, uint32_t y, const char* text, uint8_t color) {
    uint32_t i = 0;

    while (text[i] != '\0' && x + i < VGA_WIDTH) {
        screen_put_char(x + i, y, text[i], color);
        i++;
    }
}

static void screen_clear(uint8_t color) {
    uint32_t y;
    uint32_t x;

    for (y = 0; y < VGA_HEIGHT; y++) {
        for (x = 0; x < VGA_WIDTH; x++) {
            screen_put_char(x, y, ' ', color);
        }
    }
}


static void screen_fill_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint8_t color) {
    uint32_t row;
    uint32_t col;

    for (row = 0; row < height; row++) {
        for (col = 0; col < width; col++) {
            screen_put_char(x + col, y + row, ' ', color);
        }
    }
}

static void screen_hline(uint32_t x, uint32_t y, uint32_t width, char character, uint8_t color) {
    uint32_t i;

    for (i = 0; i < width; i++) {
        screen_put_char(x + i, y, character, color);
    }
}

static void screen_vline(uint32_t x, uint32_t y, uint32_t height, char character, uint8_t color) {
    uint32_t i;

    for (i = 0; i < height; i++) {
        screen_put_char(x, y + i, character, color);
    }
}

static void screen_box(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint8_t color) {
    uint32_t i;

    if (width < 2 || height < 2) {
        return;
    }

    screen_put_char(x, y, '+', color);
    screen_put_char(x + width - 1, y, '+', color);
    screen_put_char(x, y + height - 1, '+', color);
    screen_put_char(x + width - 1, y + height - 1, '+', color);

    for (i = 1; i < width - 1; i++) {
        screen_put_char(x + i, y, '-', color);
        screen_put_char(x + i, y + height - 1, '-', color);
    }

    for (i = 1; i < height - 1; i++) {
        screen_put_char(x, y + i, '|', color);
        screen_put_char(x + width - 1, y + i, '|', color);
    }
}

static void screen_put_uint(uint32_t x, uint32_t y, uint32_t value, uint8_t color) {
    char buffer[11];
    uint32_t i = 0;
    uint32_t j;

    if (value == 0) {
        screen_put_char(x, y, '0', color);
        return;
    }

    while (value > 0 && i < sizeof(buffer)) {
        buffer[i] = (char)('0' + (value % 10));
        value /= 10;
        i++;
    }

    for (j = 0; j < i; j++) {
        screen_put_char(x + j, y, buffer[i - j - 1], color);
    }
}

static void screen_put_hex8(uint32_t x, uint32_t y, uint8_t value, uint8_t color) {
    const char* hex = "0123456789ABCDEF";

    screen_put_string(x, y, "0x", color);
    screen_put_char(x + 2, y, hex[(value >> 4) & 0x0F], color);
    screen_put_char(x + 3, y, hex[value & 0x0F], color);
}

static void screen_put_bar(uint32_t x, uint32_t y, uint32_t value, uint8_t color) {
    uint32_t filled;
    uint32_t i;

    if (value > 100) {
        value = 100;
    }

    filled = value / 10;

    screen_put_char(x, y, '[', COLOR_WHITE);

    for (i = 0; i < 10; i++) {
        if (i < filled) {
            screen_put_char(x + 1 + i, y, '#', color);
        } else {
            screen_put_char(x + 1 + i, y, '-', COLOR_GRAY);
        }
    }

    screen_put_char(x + 11, y, ']', COLOR_WHITE);
}

static uint32_t random_next(void) {
    rng_state = rng_state * 1103515245U + 12345U;
    return rng_state;
}

static void speaker_enable(void) {
    uint8_t value = port_byte_in(PC_SPEAKER_PORT);

    if ((value & 0x03U) != 0x03U) {
        port_byte_out(PC_SPEAKER_PORT, (uint8_t)(value | 0x03U));
    }
}

static void speaker_disable(void) {
    uint8_t value = port_byte_in(PC_SPEAKER_PORT);

    if ((value & 0x03U) != 0U) {
        port_byte_out(PC_SPEAKER_PORT, (uint8_t)(value & 0xFCU));
    }
}

static void speaker_play(uint32_t frequency) {
    uint32_t divisor;

    if (frequency == 0U) {
        return;
    }

    divisor = PIT_BASE_FREQUENCY / frequency;

    if (divisor < 2U) {
        divisor = 2U;
    }

    if (divisor > 0xFFFEU) {
        divisor = 0xFFFEU;
    }

    divisor &= 0xFFFEU;

    port_byte_out(PIT_CMD_PORT, PIT_SPEAKER_COMMAND);
    port_byte_out(PIT_CHANNEL2_PORT, (uint8_t)(divisor & 0xFFU));
    port_byte_out(PIT_CHANNEL2_PORT, (uint8_t)((divisor >> 8) & 0xFFU));

    current_speaker_frequency = frequency;
    speaker_enable();
}

static void speaker_stop(void) {
    current_speaker_frequency = 0;
    speaker_disable();
}

static void clear_beep_queue(void) {
    beep_head = 0;
    beep_tail = 0;
    beep_count = 0;
}

static void request_beep(uint32_t frequency, uint32_t duration) {
    /*
     * Low-latency sound policy:
     * Do not queue tones. A reaction game needs immediate feedback.
     * The old queue could build up and make sounds arrive late.
     */
    clear_beep_queue();
    speaker_stop();
    speaker_stop_tick = 0;

    if (frequency == 0U || duration == 0U) {
        return;
    }

    /*
     * Keep tones short. Long tones through QEMU/WSLg/PulseAudio feel delayed.
     */
    if (duration > 120U) {
        duration = 120U;
    }

    speaker_play(frequency);
    speaker_stop_tick = pit_get_ticks() + duration;
}

static void speaker_stop_all(void) {
    speaker_stop();
    speaker_stop_tick = 0;
    clear_beep_queue();
}

static void speaker_update(uint32_t now) {
    if (speaker_stop_tick != 0U && now >= speaker_stop_tick) {
        speaker_stop();
        speaker_stop_tick = 0;
    }

    if (speaker_stop_tick == 0U && beep_count > 0U) {
        uint32_t frequency = beep_queue[beep_head].frequency;
        uint32_t duration = beep_queue[beep_head].duration;

        beep_head = (uint8_t)((beep_head + 1U) % BEEP_QUEUE_SIZE);
        beep_count--;

        if (frequency == 0U) {
            speaker_stop();
        } else {
            speaker_play(frequency);
            speaker_stop_tick = now + duration;
        }
    }
}

static void apply_difficulty(void) {
    /*
     * Smoother falling motion:
     * old values were roughly 220 / 160 / 110 ms per row.
     * These values update movement more often while keeping spawn rates controlled.
     */
    if (difficulty == 0) {
        move_delay = 150;
        spawn_delay = 950;
        return;
    }

    if (difficulty == 1) {
        move_delay = 115;
        spawn_delay = 720;
        return;
    }

    move_delay = 90;
    spawn_delay = 520;
}

static const char* difficulty_name(void) {
    if (difficulty == 0) {
        return "EASY";
    }

    if (difficulty == 1) {
        return "NORMAL";
    }

    return "PANIC";
}

static uint32_t panic_gain_normal_miss(void) {
    if (difficulty == 0) {
        return 7;
    }

    if (difficulty == 1) {
        return 10;
    }

    return 13;
}

static uint32_t panic_gain_priority_miss(void) {
    if (difficulty == 0) {
        return 12;
    }

    if (difficulty == 1) {
        return 16;
    }

    return 22;
}

static uint32_t panic_gain_wrong_press(void) {
    if (difficulty == 0) {
        return 2;
    }

    if (difficulty == 1) {
        return 3;
    }

    return 4;
}

static uint32_t panic_gain_queue_full(void) {
    if (difficulty == 0) {
        return 3;
    }

    if (difficulty == 1) {
        return 4;
    }

    return 5;
}

static uint32_t panic_relief_normal(void) {
    if (difficulty == 0) {
        return 4;
    }

    if (difficulty == 1) {
        return 3;
    }

    return 2;
}

static uint32_t panic_relief_priority(void) {
    if (difficulty == 0) {
        return 8;
    }

    if (difficulty == 1) {
        return 6;
    }

    return 5;
}

static void reduce_panic(uint32_t amount) {
    if (panic_level > amount) {
        panic_level -= amount;
    } else {
        panic_level = 0;
    }
}

static void clear_feedback(void) {
    uint32_t i;

    for (i = 0; i < 4; i++) {
        feedback_active[i] = 0;
        feedback_symbol[i] = ' ';
        feedback_color[i] = COLOR_WHITE;
        feedback_until_tick[i] = 0;
    }
}

static void show_feedback(uint8_t lane, uint8_t success, uint8_t event_type) {
    if (lane >= 4) {
        return;
    }

    feedback_active[lane] = 1;
    feedback_symbol[lane] = success ? 'V' : 'X';

    if (event_type == EVENT_PRIORITY) {
        feedback_color[lane] = COLOR_RED;
    } else {
        feedback_color[lane] = COLOR_GREEN;
    }

    feedback_until_tick[lane] = pit_get_ticks() + 420U;
    needs_render = 1;
}

static void update_feedback(uint32_t now) {
    uint32_t i;

    for (i = 0; i < 4; i++) {
        if (feedback_active[i] && now >= feedback_until_tick[i]) {
            feedback_active[i] = 0;
            feedback_symbol[i] = ' ';
            feedback_until_tick[i] = 0;
            needs_render = 1;
        }
    }
}

static void clear_events(void) {
    uint32_t i;

    for (i = 0; i < MAX_EVENTS; i++) {
        events[i].active = 0;
        events[i].lane = 0;
        events[i].row = 0;
        events[i].type = EVENT_NORMAL;
        events[i].spawn_tick = 0;
    }
}

static uint32_t active_event_count(void) {
    uint32_t count = 0;
    uint32_t i;

    for (i = 0; i < MAX_EVENTS; i++) {
        if (events[i].active) {
            count++;
        }
    }

    return count;
}

static void start_game(void) {
    uint32_t now = pit_get_ticks();

    clear_events();
    clear_feedback();

    score = 0;
    combo = 0;
    best_combo = 0;
    panic_level = 0;
    events_serviced = 0;
    events_missed = 0;

    last_move_tick = now;
    last_spawn_tick = now;
    last_render_tick = 0;

    rng_state = now ^ 0x0BADC0DEU;

    apply_difficulty();

    current_screen = SCREEN_PLAYING;
    last_action = "Game started. Match the lane and hit inside the green zone.";
    needs_render = 1;

    speaker_stop_all();
    request_beep(660, 70);
}

static void spawn_event(uint32_t now) {
    uint32_t i;
    uint32_t value;
    uint8_t lane;
    uint8_t type;
    uint32_t gain;

    for (i = 0; i < MAX_EVENTS; i++) {
        if (!events[i].active) {
            value = random_next();
            lane = (uint8_t)((value >> 16) % 4U);

            if ((value % 5U) == 0U) {
                type = EVENT_PRIORITY;
            } else {
                type = EVENT_NORMAL;
            }

            events[i].active = 1;
            events[i].lane = lane;
            events[i].row = FIRST_EVENT_ROW;
            events[i].type = type;
            events[i].spawn_tick = now;

            needs_render = 1;
            return;
        }
    }

    gain = panic_gain_queue_full();
    if (panic_level + gain < 100U) {
        panic_level += gain;
    } else {
        panic_level = 100U;
    }

    last_action = "Event queue full.";
    if (panic_level >= 100U) {
        current_screen = SCREEN_GAME_OVER;
        request_beep(90, 100);
        request_beep(70, 100);
    }
    needs_render = 1;
}

static void go_to_game_over(void) {
    current_screen = SCREEN_GAME_OVER;
    last_action = "KERNEL PANIC.";
    request_beep(90, 100);
    request_beep(70, 100);
}

static void register_miss(uint8_t type, uint8_t lane) {
    uint32_t gain;

    events_missed++;
    combo = 0;
    show_feedback(lane, 0, type);

    if (type == EVENT_PRIORITY) {
        gain = panic_gain_priority_miss();

        if (panic_level + gain < 100U) {
            panic_level += gain;
        } else {
            panic_level = 100U;
        }

        last_action = "Priority interrupt missed.";
        request_beep(140, 60);
        request_beep(110, 60);
    } else {
        gain = panic_gain_normal_miss();

        if (panic_level + gain < 100U) {
            panic_level += gain;
        } else {
            panic_level = 100U;
        }

        last_action = "Interrupt missed.";
        request_beep(180, 60);
    }

    if (panic_level >= 100U) {
        go_to_game_over();
    }

    needs_render = 1;
}

static void move_events(void) {
    uint32_t i;

    for (i = 0; i < MAX_EVENTS; i++) {
        if (events[i].active) {
            events[i].row++;

            if (events[i].row > MISS_ROW) {
                uint8_t type = events[i].type;
                uint8_t lane = events[i].lane;
                events[i].active = 0;
                register_miss(type, lane);
            }
        }
    }

    needs_render = 1;
}

static uint32_t lane_frequency(uint8_t lane) {
    if (lane == 0) {
        return 440;
    }

    if (lane == 1) {
        return 523;
    }

    if (lane == 2) {
        return 659;
    }

    return 784;
}

static uint8_t row_distance(uint8_t row) {
    if (row > HIT_ROW) {
        return (uint8_t)(row - HIT_ROW);
    }

    return (uint8_t)(HIT_ROW - row);
}

static void hit_lane(uint8_t lane) {
    uint32_t i;
    uint32_t best_index = MAX_EVENTS;
    uint8_t best_distance = 255;
    uint32_t relief;

    for (i = 0; i < MAX_EVENTS; i++) {
        if (events[i].active && events[i].lane == lane) {
            uint8_t distance = row_distance(events[i].row);

            if (distance <= HIT_WINDOW && distance < best_distance) {
                best_distance = distance;
                best_index = i;
            }
        }
    }

    if (best_index != MAX_EVENTS) {
        uint8_t type = events[best_index].type;

        show_feedback(lane, 1, type);
        events[best_index].active = 0;
        events_serviced++;
        combo++;

        if (combo > best_combo) {
            best_combo = combo;
        }

        if (type == EVENT_PRIORITY) {
            score += 250U + (combo * 10U);
            last_action = "Priority interrupt serviced.";
            relief = panic_relief_priority();

            /* Hit sound removed: visual feedback is immediate and avoids QEMU audio delay. */
            /* Hit sound removed: visual feedback is immediate and avoids QEMU audio delay. */
        } else {
            score += 100U + (combo * 5U);
            last_action = "Interrupt serviced.";
            relief = panic_relief_normal();

            /* Hit sound removed: visual feedback is immediate and avoids QEMU audio delay. */
        }

        if (combo >= 3U) {
            relief += 2U;
        }

        reduce_panic(relief);

        needs_render = 1;
        return;
    }

    combo = 0;

    relief = panic_gain_wrong_press();
    if (panic_level + relief < 100U) {
        panic_level += relief;
    } else {
        panic_level = 100U;
    }

    last_action = "Wrong service window.";
    show_feedback(lane, 0, EVENT_PRIORITY);
    request_beep(190, 50);

    if (panic_level >= 100U) {
        go_to_game_over();
    }

    needs_render = 1;
}

static void render_header(const char* title) {
    screen_hline(0, 0, VGA_WIDTH, '=', COLOR_CYAN);
    screen_put_string(2, 1, title, COLOR_YELLOW);
    screen_put_string(64, 1, "IRQ Rush", COLOR_CYAN);
    screen_hline(0, 2, VGA_WIDTH, '=', COLOR_CYAN);
}

static void render_holo_frame(uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
    uint32_t phase = (pit_get_ticks() / 8U);
    uint32_t run_top;
    uint32_t run_bottom;
    uint32_t run_side;

    screen_box(x, y, width, height, COLOR_CYAN);

    if (width > 4 && height > 4) {
        screen_box(x + 1, y + 1, width - 2, height - 2, (phase & 1U) ? COLOR_MAGENTA : COLOR_GRAY);
    }

    if (width > 6) {
        run_top = (phase % (width - 4U));
        run_bottom = ((phase + (width / 3U)) % (width - 4U));

        screen_put_char(x + 2 + run_top, y, '*', COLOR_WHITE);
        screen_put_char(x + 2 + run_bottom, y + height - 1, '*', COLOR_CYAN);
    }

    if (height > 6) {
        run_side = (phase % (height - 4U));
        screen_put_char(x, y + 2 + run_side, '*', COLOR_MAGENTA);
        screen_put_char(x + width - 1, y + 2 + ((run_side + 3U) % (height - 4U)), '*', COLOR_WHITE);
    }

    screen_put_char(x + 1, y + 1, '+', COLOR_WHITE);
    screen_put_char(x + width - 2, y + 1, '+', COLOR_WHITE);
    screen_put_char(x + 1, y + height - 2, '+', COLOR_WHITE);
    screen_put_char(x + width - 2, y + height - 2, '+', COLOR_WHITE);
}

static void render_menu_background(void) {
    uint32_t phase = pit_get_ticks() / 5U;
    uint32_t x;
    uint32_t trail;

    static const char rain_chars[] = {
        '0', '1', 'A', 'S', 'D', 'F', 'I', 'R',
        'Q', 'P', 'I', 'T', 'V', 'G', 'A', '#'
    };

    screen_clear(COLOR_WHITE);

    /*
     * Pink Matrix rain only outside the central menu.
     * Left side:  columns 1-13
     * Right side: columns 66-78
     * The center is kept clean so the menu stays readable.
     */

    for (x = 1; x <= 13; x += 2) {
        uint32_t head = 3U + ((phase + (x * 7U)) % 21U);

        for (trail = 0; trail < 8; trail++) {
            uint32_t y;
            uint32_t index;
            uint8_t color;

            if (head < trail) {
                continue;
            }

            y = head - trail;

            if (y < 3U || y > 23U) {
                continue;
            }

            index = (phase + x + trail * 5U) % sizeof(rain_chars);

            if (trail == 0U) {
                color = COLOR_WHITE;
            } else if (trail < 3U) {
                color = COLOR_MAGENTA;
            } else if (trail < 6U) {
                color = COLOR_RED;
            } else {
                color = COLOR_GRAY;
            }

            screen_put_char(x, y, rain_chars[index], color);
        }
    }

    for (x = 66; x <= 78; x += 2) {
        uint32_t head = 3U + ((phase + (x * 5U)) % 21U);

        for (trail = 0; trail < 8; trail++) {
            uint32_t y;
            uint32_t index;
            uint8_t color;

            if (head < trail) {
                continue;
            }

            y = head - trail;

            if (y < 3U || y > 23U) {
                continue;
            }

            index = (phase + x + trail * 3U) % sizeof(rain_chars);

            if (trail == 0U) {
                color = COLOR_WHITE;
            } else if (trail < 3U) {
                color = COLOR_MAGENTA;
            } else if (trail < 6U) {
                color = COLOR_RED;
            } else {
                color = COLOR_GRAY;
            }

            screen_put_char(x, y, rain_chars[index], color);
        }
    }

    screen_hline(0, 0, VGA_WIDTH, '=', COLOR_MAGENTA);
    screen_hline(0, 2, VGA_WIDTH, '=', COLOR_MAGENTA);
    screen_hline(0, 24, VGA_WIDTH, '=', COLOR_MAGENTA);
}

static void render_menu_status_panel(void) {
    screen_box(45, 6, 20, 13, COLOR_CYAN);

    screen_put_string(48, 7, "SYSTEM STATUS", COLOR_YELLOW);
    screen_put_string(47, 9,  "PIT:      ONLINE", COLOR_GREEN);
    screen_put_string(47, 10, "IRQ:      ONLINE", COLOR_GREEN);
    screen_put_string(47, 11, "VGA:      ONLINE", COLOR_GREEN);
    screen_put_string(47, 12, "PCSPK:    READY ", COLOR_GREEN);

    screen_put_string(47, 14, "MODE:", COLOR_YELLOW);
    screen_put_string(53, 14, difficulty_name(), COLOR_WHITE);

    screen_put_string(47, 15, "SPAWN:", COLOR_YELLOW);
    screen_put_uint(54, 15, spawn_delay, COLOR_WHITE);
    screen_put_string(58, 15, "ms", COLOR_GRAY);

    screen_put_string(47, 16, "MOVE:", COLOR_YELLOW);
    screen_put_uint(53, 16, move_delay, COLOR_WHITE);
    screen_put_string(57, 16, "ms", COLOR_GRAY);
}

static void render_menu(void) {
    uint32_t blink = (pit_get_ticks() / 20U) & 1U;

    render_menu_background();
    screen_put_string(29, 1, "IRQ RUSH: KERNEL PANIC", COLOR_YELLOW);
    /*
     * Central clean menu panel.
     * Rain stays outside this box, like the reference image.
     */
    screen_fill_rect(17, 4, 46, 18, COLOR_WHITE);
    screen_box(16, 3, 48, 20, COLOR_MAGENTA);
    screen_box(17, 4, 46, 18, COLOR_CYAN);

    screen_put_string(25, 5, "INTERRUPT CONSOLE", COLOR_MAGENTA);
    screen_put_string(21, 6, "Service the falling IRQ load", COLOR_GRAY);

    screen_hline(20, 8, 40, '-', COLOR_GRAY);

    screen_put_string(23, 9,  "1. Start game", COLOR_GREEN);
    screen_put_string(23, 11, "2. Difficulty: ", COLOR_WHITE);
    screen_put_string(37, 11, difficulty_name(), COLOR_YELLOW);
    screen_put_string(23, 13, "3. Speaker test", COLOR_WHITE);
    screen_put_string(23, 15, "4. Diagnostics", COLOR_WHITE);
    screen_put_string(23, 17, "5. Credits", COLOR_WHITE);
    screen_put_string(23, 19, "6. Help / Rules", COLOR_WHITE);

    screen_hline(20, 20, 40, '-', COLOR_GRAY);

    screen_put_string(21, 21, "PIT:", COLOR_YELLOW);
    screen_put_string(26, 21, "ONLINE", COLOR_GREEN);

    screen_put_string(35, 21, "IRQ:", COLOR_YELLOW);
    screen_put_string(40, 21, "ONLINE", COLOR_GREEN);

    screen_put_string(49, 21, "PCSPK:", COLOR_YELLOW);
    screen_put_string(56, 21, "READY", COLOR_GREEN);

    if (blink == 0U) {
        screen_put_string(12, 23, "A/S/D/F or 1/2/3/4. ESC goes back to menu.", COLOR_CYAN);
    } else {
        screen_put_string(14, 23, "Incoming interrupts detected outside the console...", COLOR_WHITE);
    }
}

static void render_lanes(void) {
    uint32_t lane;
    uint32_t y;

    for (lane = 0; lane < 4; lane++) {
        uint8_t x = lane_x[lane];

        screen_put_string((uint32_t)(x - 3), 3, lane_names[lane], COLOR_YELLOW);

        for (y = FIRST_EVENT_ROW; y <= MISS_ROW; y++) {
            screen_put_char(x, y, '|', COLOR_GRAY);
        }

        /*
         * Small service bay around the hit zone and feedback.
         * This keeps the V/X area separated from the status text.
         */
        screen_box((uint32_t)(x - 3), 19, 7, 4, COLOR_GRAY);

        screen_put_char((uint32_t)(x - 2), HIT_ROW, '[', COLOR_YELLOW);
        screen_put_char((uint32_t)(x - 1), HIT_ROW, '=', COLOR_YELLOW);
        screen_put_char(x, HIT_ROW, '=', COLOR_YELLOW);
        screen_put_char((uint32_t)(x + 1), HIT_ROW, '=', COLOR_YELLOW);
        screen_put_char((uint32_t)(x + 2), HIT_ROW, ']', COLOR_YELLOW);
    }

    screen_put_string(1, 4, "Green [!] = normal", COLOR_GREEN);
    screen_put_string(23, 4, "Red [#] = priority", COLOR_RED);

    screen_put_string(1, 24, "A/1 TIMER", COLOR_CYAN);
    screen_put_string(16, 24, "S/2 KEYBOARD", COLOR_CYAN);
    screen_put_string(35, 24, "D/3 MEMORY", COLOR_CYAN);
    screen_put_string(52, 24, "F/4 SOUND", COLOR_CYAN);
}

static void render_event_marker(uint8_t x, uint8_t row, uint8_t type) {
    uint8_t color;
    char symbol;

    if (type == EVENT_PRIORITY) {
        color = COLOR_RED;
        symbol = '!';
    } else {
        color = COLOR_GREEN;
        symbol = '?';
    }

    /*
     * Main marker.
     * Wider than the old marker so it is easier to track while falling.
     */
    screen_put_char((uint32_t)(x - 2), row, '<', color);
    screen_put_char((uint32_t)(x - 1), row, '[', color);
    screen_put_char(x, row, symbol, color);
    screen_put_char((uint32_t)(x + 1), row, ']', color);
    screen_put_char((uint32_t)(x + 2), row, '>', color);

    /*
     * Small trail above the event.
     * This makes the falling movement feel smoother in VGA text mode.
     */
    if (row > FIRST_EVENT_ROW) {
        screen_put_char(x, (uint32_t)(row - 1U), '|', COLOR_GRAY);
    }

    if (row > FIRST_EVENT_ROW + 1U) {
        screen_put_char(x, (uint32_t)(row - 2U), '.', COLOR_GRAY);
    }
}

static void render_events(void) {
    uint32_t i;

    for (i = 0; i < MAX_EVENTS; i++) {
        if (events[i].active) {
            render_event_marker(lane_x[events[i].lane], events[i].row, events[i].type);
        }
    }
}


static void render_feedback(void) {
    uint32_t i;
    uint32_t now = pit_get_ticks();

    for (i = 0; i < 4; i++) {
        if (feedback_active[i]) {
            uint8_t x = lane_x[i];
            uint8_t row = FEEDBACK_ROW;
            uint8_t color = feedback_color[i];

            /*
             * Draw feedback under the yellow target zone.
             * V = success, X = failed/missed/wrong press.
             */
            if (((now / 80U) % 2U) == 0U) {
                screen_put_char((uint32_t)(x - 2), row, '<', color);
                screen_put_char((uint32_t)(x - 1), row, '[', color);
                screen_put_char(x, row, feedback_symbol[i], color);
                screen_put_char((uint32_t)(x + 1), row, ']', color);
                screen_put_char((uint32_t)(x + 2), row, '>', color);
            } else {
                screen_put_char((uint32_t)(x - 1), row, '[', color);
                screen_put_char(x, row, feedback_symbol[i], color);
                screen_put_char((uint32_t)(x + 1), row, ']', color);
            }
        }
    }
}

static void render_playing(void) {
    screen_clear(COLOR_WHITE);
    render_header("LIVE INTERRUPT LOAD");

    render_lanes();
    render_events();
    render_feedback();

    screen_box(63, 5, 16, 16, COLOR_CYAN);

    screen_put_string(65, 6, "SCORE", COLOR_YELLOW);
    screen_put_uint(65, 7, score, COLOR_WHITE);

    screen_put_string(65, 9, "COMBO", COLOR_YELLOW);
    screen_put_uint(65, 10, combo, COLOR_WHITE);

    screen_put_string(65, 12, "BEST", COLOR_YELLOW);
    screen_put_uint(65, 13, best_combo, COLOR_WHITE);

    screen_put_string(65, 15, "PANIC", COLOR_YELLOW);
    screen_put_bar(65, 16, panic_level, panic_level >= 70U ? COLOR_RED : COLOR_GREEN);

    screen_put_string(65, 18, "TICKS", COLOR_YELLOW);
    screen_put_uint(65, 19, pit_get_ticks(), COLOR_WHITE);

    screen_put_string(1, 23, "STATUS:", COLOR_YELLOW);
    screen_put_string(9, 23, last_action, COLOR_WHITE);
}

static void render_game_over(void) {
    /*
     * Game over is a separate screen.
     * Clear the old gameplay screen first so lanes and HUD do not remain behind it.
     */
    screen_clear(COLOR_WHITE);
    render_header("KERNEL PANIC");

    /*
     * Wider red panic window so the backlog message stays inside the border.
     * This only affects the death/game-over screen.
     */
    screen_box(8, 7, 66, 10, COLOR_RED);

    screen_put_string(31, 9, "*** KERNEL PANIC ***", COLOR_RED);

    screen_put_string(18, 11, "The interrupt backlog overloaded the system.", COLOR_WHITE);

    screen_put_string(24, 13, "Final score:", COLOR_YELLOW);
    screen_put_uint(38, 13, score, COLOR_WHITE);

    screen_put_string(24, 15, "Best combo:", COLOR_YELLOW);
    screen_put_uint(38, 15, best_combo, COLOR_WHITE);

    screen_put_string(23, 17, "Press R to restart or ESC for menu.", COLOR_CYAN);
}

static void render_diagnostics(void) {
    screen_clear(COLOR_WHITE);
    render_header("DIAGNOSTICS");

    screen_box(12, 5, 56, 16, COLOR_CYAN);

    screen_put_string(17, 7, "PIT ticks:              ", COLOR_YELLOW);
    screen_put_uint(42, 7, pit_get_ticks(), COLOR_WHITE);

    screen_put_string(17, 9, "Last scancode:          ", COLOR_YELLOW);
    screen_put_hex8(42, 9, last_scancode, COLOR_WHITE);

    screen_put_string(17, 11, "Active events:          ", COLOR_YELLOW);
    screen_put_uint(42, 11, active_event_count(), COLOR_WHITE);
    screen_put_string(45, 11, "/", COLOR_GRAY);
    screen_put_uint(46, 11, MAX_EVENTS, COLOR_WHITE);

    screen_put_string(17, 13, "Speaker frequency:      ", COLOR_YELLOW);
    screen_put_uint(42, 13, current_speaker_frequency, COLOR_WHITE);
    screen_put_string(47, 13, "Hz", COLOR_GRAY);

    screen_put_string(17, 15, "Serviced / missed:      ", COLOR_YELLOW);
    screen_put_uint(42, 15, events_serviced, COLOR_GREEN);
    screen_put_string(48, 15, "/", COLOR_GRAY);
    screen_put_uint(50, 15, events_missed, COLOR_RED);

    screen_put_string(17, 17, "Difficulty:             ", COLOR_YELLOW);
    screen_put_string(42, 17, difficulty_name(), COLOR_WHITE);

    screen_put_string(17, 19, "ESC or Q returns to the menu.", COLOR_CYAN);
}

static void render_speaker_test(void) {
    screen_clear(COLOR_WHITE);
    render_header("SPEAKER TEST");

    screen_box(16, 6, 48, 13, COLOR_CYAN);

    screen_put_string(23, 8, "Press 1: TIMER tone", COLOR_WHITE);
    screen_put_string(23, 10, "Press 2: KEYBOARD tone", COLOR_WHITE);
    screen_put_string(23, 12, "Press 3: MEMORY tone", COLOR_WHITE);
    screen_put_string(23, 14, "Press 4: SOUND tone", COLOR_WHITE);

    screen_put_string(23, 17, "ESC or Q returns to menu.", COLOR_CYAN);
}

static void render_credits(void) {
    screen_clear(COLOR_WHITE);
    render_header("CREDITS");

    screen_box(10, 6, 60, 13, COLOR_CYAN);

    screen_put_string(18, 8, "IRQ Rush: Kernel Panic", COLOR_YELLOW);
    screen_put_string(18, 10, "An OS-themed reaction game for Assignment 6.", COLOR_WHITE);
    screen_put_string(18, 12, "Uses PIT timing, IRQ keyboard input,", COLOR_WHITE);
    screen_put_string(18, 13, "VGA text rendering and PC speaker audio.", COLOR_WHITE);
    screen_put_string(18, 16, "ESC or Q returns to menu.", COLOR_CYAN);
}

static void render_help(void) {
    /*
     * Help screen must be a clean standalone screen.
     * Do not draw the Matrix/menu background here.
     */
    screen_clear(COLOR_WHITE);
    render_header("HELP / RULES");

    screen_box(2, 4, 76, 20, COLOR_CYAN);
    screen_box(3, 5, 74, 18, COLOR_MAGENTA);

    screen_put_string(6, 6,  "Goal:", COLOR_YELLOW);
    screen_put_string(14, 6, "Keep panic below 100% by servicing falling interrupts.", COLOR_WHITE);

    screen_put_string(6, 8,  "Objects:", COLOR_YELLOW);
    screen_put_string(17, 8, "Green [!] = normal. Red [#] = priority.", COLOR_WHITE);

    screen_put_string(6, 10, "Controls:", COLOR_YELLOW);
    screen_put_string(18, 10, "A/1 TIMER   S/2 KEYBOARD   D/3 MEMORY   F/4 SOUND", COLOR_WHITE);

    screen_put_string(6, 12, "Hit zone:", COLOR_YELLOW);
    screen_put_string(18, 12, "Press when event reaches the yellow [===] target.", COLOR_WHITE);

    screen_put_string(6, 14, "Panic:", COLOR_YELLOW);
    screen_put_string(15, 14, "Hits lower panic. Combos lower it more.", COLOR_WHITE);
    screen_put_string(15, 15, "Misses and wrong presses raise panic.", COLOR_WHITE);
    screen_put_string(15, 16, "Priority misses hurt more than normal misses.", COLOR_WHITE);

    screen_put_string(6, 18, "Difficulty:", COLOR_YELLOW);
    screen_put_string(18, 18, "Easy   move 150ms | spawn 950ms | gains +7/+12/+2", COLOR_WHITE);
    screen_put_string(18, 19, "Normal move 115ms | spawn 720ms | gains +10/+16/+3", COLOR_WHITE);
    screen_put_string(18, 20, "Panic  move  90ms | spawn 520ms | gains +13/+22/+4", COLOR_WHITE);

    screen_put_string(9, 22, "Gains: normal miss / priority miss / wrong press.", COLOR_CYAN);
    screen_put_string(26, 23, "ESC or Q returns to menu.", COLOR_CYAN);
}

void irq_rush_init(void) {
    apply_difficulty();
    clear_events();

    current_screen = SCREEN_MENU;
    needs_render = 1;
    last_action = "System ready.";

    speaker_stop_all();
}

void irq_rush_update(void) {
    uint32_t now = pit_get_ticks();

    speaker_update(now);
    update_feedback(now);

    if (current_screen != SCREEN_PLAYING) {
        return;
    }

    if (now - last_move_tick >= move_delay) {
        move_events();
        last_move_tick = now;
    }

    if (now - last_spawn_tick >= spawn_delay) {
        spawn_event(now);
        last_spawn_tick = now;
    }
}

void irq_rush_render(void) {
    uint32_t now = pit_get_ticks();
    uint32_t refresh_limit = 25U;

    if (current_screen == SCREEN_MENU) {
        refresh_limit = 80U;
    } else if (current_screen == SCREEN_HELP) {
        refresh_limit = 100U;
    }

    if (!needs_render && now - last_render_tick < refresh_limit) {
        return;
    }

    last_render_tick = now;
    needs_render = 0;

    if (current_screen == SCREEN_MENU) {
        render_menu();
    } else if (current_screen == SCREEN_PLAYING) {
        render_playing();
    } else if (current_screen == SCREEN_DIAGNOSTICS) {
        render_diagnostics();
    } else if (current_screen == SCREEN_SPEAKER_TEST) {
        render_speaker_test();
    } else if (current_screen == SCREEN_CREDITS) {
        render_credits();
    } else if (current_screen == SCREEN_HELP) {
        render_help();
    } else {
        render_game_over();
    }
}

void irq_rush_handle_scancode(uint8_t scancode) {
    last_scancode = scancode;
    needs_render = 1;

    if (scancode == 0xE0U || scancode == 0xE1U) {
        return;
    }

    if ((scancode & 0x80U) != 0U) {
        return;
    }

    if (current_screen == SCREEN_MENU) {
        if (scancode == SC_1) {
            start_game();
        } else if (scancode == SC_2) {
            difficulty = (uint8_t)((difficulty + 1U) % 3U);
            apply_difficulty();
            last_action = "Difficulty changed.";
            request_beep(520, 50);
        } else if (scancode == SC_3) {
            current_screen = SCREEN_SPEAKER_TEST;
            request_beep(660, 50);
        } else if (scancode == SC_4) {
            current_screen = SCREEN_DIAGNOSTICS;
            request_beep(600, 50);
        } else if (scancode == SC_5) {
            current_screen = SCREEN_CREDITS;
            request_beep(560, 50);
        } else if (scancode == SC_6) {
            current_screen = SCREEN_HELP;
            request_beep(620, 50);
        }

        return;
    }

    if (current_screen == SCREEN_PLAYING) {
        if (scancode == SC_ESCAPE) {
            current_screen = SCREEN_MENU;
            speaker_stop_all();
            return;
        }

        if (scancode == SC_A || scancode == SC_1) {
            hit_lane(0);
        } else if (scancode == SC_S || scancode == SC_2) {
            hit_lane(1);
        } else if (scancode == SC_D || scancode == SC_3) {
            hit_lane(2);
        } else if (scancode == SC_F || scancode == SC_4) {
            hit_lane(3);
        }

        return;
    }

    if (current_screen == SCREEN_GAME_OVER) {
        if (scancode == SC_R) {
            start_game();
        } else if (scancode == SC_ESCAPE || scancode == SC_Q) {
            current_screen = SCREEN_MENU;
            speaker_stop_all();
        }

        return;
    }

    if (current_screen == SCREEN_SPEAKER_TEST) {
        if (scancode == SC_1) {
            request_beep(lane_frequency(0), 140);
        } else if (scancode == SC_2) {
            request_beep(lane_frequency(1), 140);
        } else if (scancode == SC_3) {
            request_beep(lane_frequency(2), 140);
        } else if (scancode == SC_4) {
            request_beep(lane_frequency(3), 140);
        } else if (scancode == SC_ESCAPE || scancode == SC_Q) {
            current_screen = SCREEN_MENU;
            speaker_stop_all();
        }

        return;
    }

    if (scancode == SC_ESCAPE || scancode == SC_Q) {
        current_screen = SCREEN_MENU;
        speaker_stop_all();
    }
}
