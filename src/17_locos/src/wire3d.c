#include "wire3d.h"
#include "irq.h"
#include "pcspk.h"
#include "pit.h"
#include "terminal.h"
#include <libc/stdint.h>
#include <libc/stdbool.h>

// 3D point data for the cube
struct vec3 {
    float x;
    float y;
    float z;
};

// 2D point data after projection
struct vec2 {
    float x;
    float y;
};

// The cube corners in 3D space
static const struct vec3 cube_vertices[8] = {
    { -1.0f, -1.0f, -1.0f }, { -1.0f,  1.0f, -1.0f },
    {  1.0f,  1.0f, -1.0f }, {  1.0f, -1.0f, -1.0f },
    {  1.0f,  1.0f,  1.0f }, {  1.0f, -1.0f,  1.0f },
    { -1.0f, -1.0f,  1.0f }, { -1.0f,  1.0f,  1.0f }
};

// Each triangle uses three vertex indexes
static const uint8_t cube_triangles[12][3] = {
    { 0, 1, 2 }, { 0, 2, 3 },
    { 3, 2, 4 }, { 3, 4, 5 },
    { 5, 4, 7 }, { 5, 7, 6 },
    { 6, 7, 1 }, { 6, 1, 0 },
    { 6, 0, 3 }, { 6, 3, 5 },
    { 1, 7, 4 }, { 1, 4, 2 }
};

/* One symbol pair per face, like the original approach. */
static const char face_symbols[12] = {
    '$', '$', '*', '*', '+', '+', '-', '-', '@', '@', '=', '='
};

// Camera looks toward the cube from the front
static const struct vec3 camera_v = { 0.0f, 0.0f, -1.0f };

// Small integer absolute value helper
static int iabs(int v) {
    return (v < 0) ? -v : v;
}

// Round a float to the nearest int
static int iround(float x) {
    if (x >= 0.0f) return (int)(x + 0.5f);
    return (int)(x - 0.5f);
}

// Keep angles inside the normal pi range
static float wrap_pi(float x) {
    const float pi = 3.14159265f;
    const float two_pi = 6.28318530f;
    while (x > pi) x -= two_pi;
    while (x < -pi) x += two_pi;
    return x;
}

/*
 * Range-reduced sine approximation:
 * 1) wrap to [-pi, pi]
 * 2) fold to [-pi/2, pi/2] where low-order polynomial is stable
 * This avoids the visible "flip/jump" artifacts from coarse global Taylor use.
 */
static float fast_sin(float x) {
    x = wrap_pi(x);
    const float half_pi = 1.57079632f;
    const float pi = 3.14159265f;

    if (x > half_pi) {
        x = pi - x;
    } else if (x < -half_pi) {
        x = -pi - x;
    }

    float x2 = x * x;
    float x3 = x * x2;
    float x5 = x3 * x2;
    float x7 = x5 * x2;
    /* x - x^3/3! + x^5/5! - x^7/7! */
    return x - (x3 * (1.0f / 6.0f)) + (x5 * (1.0f / 120.0f)) - (x7 * (1.0f / 5040.0f));
}

// Cosine uses the sine helper with a phase shift
static float fast_cos(float x) {
    return fast_sin(x + 1.57079632f);
}

// Rotate one point around X
static struct vec3 rotate_x(struct vec3 v, float a) {
    float c = fast_cos(a), s = fast_sin(a);
    return (struct vec3){ v.x, c * v.y - s * v.z, s * v.y + c * v.z };
}

// Rotate one point around Y
static struct vec3 rotate_y(struct vec3 v, float a) {
    float c = fast_cos(a), s = fast_sin(a);
    return (struct vec3){ c * v.x + s * v.z, v.y, -s * v.x + c * v.z };
}

// Rotate one point around Z
static struct vec3 rotate_z(struct vec3 v, float a) {
    float c = fast_cos(a), s = fast_sin(a);
    return (struct vec3){ c * v.x - s * v.y, s * v.x + c * v.y, v.z };
}

// Dot product for face visibility
static float dot_product(struct vec3 a, struct vec3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

// Cross product for face normals
static struct vec3 cross_product(struct vec3 a, struct vec3 b) {
    return (struct vec3){
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

// Project 3D points to the text grid
static struct vec2 project(struct vec3 v, int cols, int rows) {
    return (struct vec2){
        (float)iround(v.x / v.z + ((float)cols * 0.5f)),
        (float)iround(v.y / v.z + ((float)rows * 0.5f))
    };
}

// Put one character on the screen if it is inside the bounds
static inline void put_cell(int x, int y, char ch, int cols, int rows) {
    if (x < 0 || x >= cols || y < 1 || y >= rows) return; /* keep top row for title */
    terminal_put_at(x, y, ch);
}

// Fill one horizontal line of a triangle
static void draw_scanline(int y, int x0, int x1, char symbol, int cols, int rows) {
    if (y < 1 || y >= rows) return;

    int left = x0;
    int right = x1;
    if (left > right) {
        int t = left;
        left = right;
        right = t;
    }
    if (right < 0 || left >= cols) return;
    if (left < 0) left = 0;
    if (right >= cols) right = cols - 1;

    for (int x = left; x <= right; x++) {
        put_cell(x, y, symbol, cols, rows);
    }
}

// Draw a triangle with a flat top
static void draw_flat_top(struct vec2 t0, struct vec2 t1, struct vec2 b, char symbol, int cols, int rows) {
    float xb = t0.x;
    float xe = t1.x;
    float inc0 = (b.x - t0.x) / (b.y - t0.y);
    float inc1 = (b.x - t1.x) / (b.y - t1.y);

    int yb = (int)t0.y;
    int ye = (int)(b.y + 1.0f);
    for (int y = yb; y < ye; y++) {
        draw_scanline(y, (int)xb, (int)xe, symbol, cols, rows);
        xb += inc0;
        xe += inc1;
    }
}

// Draw a triangle with a flat bottom
static void draw_flat_bottom(struct vec2 t, struct vec2 b0, struct vec2 b1, char symbol, int cols, int rows) {
    float xb = t.x;
    float xe = t.x;
    float dec0 = (t.x - b0.x) / (b0.y - t.y);
    float dec1 = (t.x - b1.x) / (b1.y - t.y);

    int yb = (int)t.y;
    int ye = (int)(b0.y + 1.0f);
    for (int y = yb; y < ye; y++) {
        draw_scanline(y, (int)xb, (int)xe, symbol, cols, rows);
        xb -= dec0;
        xe -= dec1;
    }
}

// Split any triangle into simple scanline parts
static void draw_triangle(struct vec2 v0, struct vec2 v1, struct vec2 v2, char symbol, int cols, int rows) {
    struct vec2 a = v0, b = v1, c = v2;
    struct vec2 tmp;

    if (a.y > b.y) { tmp = a; a = b; b = tmp; }
    if (b.y > c.y) { tmp = b; b = c; c = tmp; }
    if (a.y > b.y) { tmp = a; a = b; b = tmp; }

    if ((int)b.y == (int)c.y) {
        draw_flat_bottom(a, b, c, symbol, cols, rows);
        return;
    }
    if ((int)a.y == (int)b.y) {
        draw_flat_top(a, b, c, symbol, cols, rows);
        return;
    }

    struct vec2 m = {
        a.x + (c.x - a.x) * ((b.y - a.y) / (c.y - a.y)),
        b.y
    };
    draw_flat_bottom(a, b, m, symbol, cols, rows);
    draw_flat_top(b, m, c, symbol, cols, rows);
}

/* Optional wire overlay to reinforce cubic look on coarse 80x25 grid. */
// Draw one edge with Bresenham style steps
static void draw_line(int x0, int y0, int x1, int y1, char ch, int cols, int rows) {
    int dx = iabs(x1 - x0);
    int sx = x0 < x1 ? 1 : -1;
    int dy = -iabs(y1 - y0);
    int sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;

    for (;;) {
        put_cell(x0, y0, ch, cols, rows);
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

// Run the cube demo until ESC is pressed
bool wire3d_run_demo(void) {
    // Short background melody for the demo
    static const struct note popcorn_notes[] = {
        { 880, 145 }, { 784, 145 }, { 880, 145 }, { 659, 145 }, { 523, 145 }, { 659, 145 }, { 440, 290 },
        { 880, 145 }, { 784, 145 }, { 880, 145 }, { 659, 145 }, { 523, 145 }, { 659, 145 }, { 440, 290 },
        { 880, 145 }, { 988, 145 }, { 1047, 145 }, { 988, 145 }, { 1047, 145 }, { 880, 145 }, { 988, 145 },
        { 880, 145 }, { 988, 145 }, { 784, 145 }, { 880, 145 }, { 784, 145 }, { 880, 145 }, { 698, 145 }, { 880, 290 },
        { 880, 145 }, { 784, 145 }, { 880, 145 }, { 659, 145 }, { 523, 145 }, { 659, 145 }, { 440, 290 },
        { 880, 145 }, { 784, 145 }, { 880, 145 }, { 659, 145 }, { 523, 145 }, { 659, 145 }, { 440, 290 },
        { 880, 145 }, { 988, 145 }, { 1047, 145 }, { 988, 145 }, { 1047, 145 }, { 880, 145 }, { 988, 145 },
        { 880, 145 }, { 988, 145 }, { 784, 145 }, { 880, 145 }, { 784, 145 }, { 880, 145 }, { 698, 145 }, { 880, 290 }
    };
    static const struct song popcorn_song = {
        .notes = popcorn_notes,
        .note_count = (uint32_t)(sizeof(popcorn_notes) / sizeof(popcorn_notes[0])),
        .name = "Popcorn Theme (BG)"
    };

    // Make the animation loop cleanly
    const uint32_t loop_frames = 600u; /* exact loop length */
    const float two_pi = 6.28318530f;
    uint32_t frame = 0u;
    float rx = 0.0f, ry = 0.0f, rz = 0.0f;
    int cols = terminal_width();
    int rows = terminal_height();

    // Start background music before the loop
    pcspk_bg_start(&popcorn_song, 1);

    for (;;) {
        // ESC exits the demo
        int key = kbd_try_getchar();
        if (key == 27) break;

        // Keep music moving while the cube draws
        pcspk_bg_update();
        terminal_clear();
        terminal_home();
        terminal_printf("ASCII 3D cube + Popcorn BG - ESC to exit");

        // Use the frame number to keep the motion steady
        /* Recompute from frame index to avoid drift; loops perfectly at loop_frames. */
        float phase = (two_pi * (float)frame) / (float)loop_frames;
        rx = phase * 2.0f; /* 2 full turns per loop */
        ry = phase * 3.0f; /* 3 full turns per loop */
        rz = phase * 1.0f; /* 1 full turn  per loop */

        // Store the rotated cube here
        struct vec3 transformed[8];
        // Store the screen positions here
        struct vec2 projected[8];

        for (int i = 0; i < 8; i++) {
            // Rotate every corner of the cube
            struct vec3 v = cube_vertices[i];
            v = rotate_x(v, rx);
            v = rotate_y(v, ry);
            v = rotate_z(v, rz);
            v.z += 7.0f;

            /* Keep same idea as original, but tuned for 80x25 cells. */
            v.y *= 34.0f;
            v.x *= 90.0f;

            transformed[i] = v;
            // Convert 3D position to text screen coordinates
            projected[i] = project(v, cols, rows);
        }

        for (int t = 0; t < 12; t++) {
            // Read the triangle corners
            uint8_t i0 = cube_triangles[t][0];
            uint8_t i1 = cube_triangles[t][1];
            uint8_t i2 = cube_triangles[t][2];

            // Build the face normal
            struct vec3 v01 = {
                transformed[i1].x - transformed[i0].x,
                transformed[i1].y - transformed[i0].y,
                transformed[i1].z - transformed[i0].z
            };
            struct vec3 v02 = {
                transformed[i2].x - transformed[i0].x,
                transformed[i2].y - transformed[i0].y,
                transformed[i2].z - transformed[i0].z
            };
            struct vec3 normal = cross_product(v01, v02);
            // Skip faces that point away from the camera
            if (dot_product(normal, camera_v) <= 0.0f) continue;

            // Fill the visible face with a symbol
            draw_triangle(projected[i0], projected[i1], projected[i2], face_symbols[t], cols, rows);
        }

        /* Draw cube edges on top for clearly cubic silhouette. */
        // Draw the wire outline last
        static const uint8_t edges[12][2] = {
            {0,1}, {1,2}, {2,3}, {3,0},
            {4,5}, {5,6}, {6,7}, {7,4},
            {0,6}, {1,7}, {2,4}, {3,5}
        };
        for (int e = 0; e < 12; e++) {
            uint8_t a = edges[e][0];
            uint8_t b = edges[e][1];
            draw_line((int)projected[a].x, (int)projected[a].y,
                      (int)projected[b].x, (int)projected[b].y,
                      '#', cols, rows);
        }

        // Advance the frame and wait a bit
        frame = (frame + 1u) % loop_frames;
        sleep_interrupt(33);
    }

    // Stop music when the demo ends
    pcspk_bg_stop();
    terminal_clear();
    terminal_home();
    terminal_printf("Exit ASCII 3D cube demo\n");
    return true;
}