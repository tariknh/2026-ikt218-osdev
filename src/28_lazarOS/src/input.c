#include <input.h>
#include <libc/stdio.h>

/*
 * Keyboard input ring buffer.
 *
 * We place the buffer and its indices at a FIXED physical address
 * (0x80000) so that both the IRQ handler and normal code access
 * the exact same memory.  This avoids issues with -fPIE where
 * the compiler may resolve global variables to different locations
 * depending on the calling context.
 *
 * Memory map: 0x80000 is in conventional memory, well below our
 * kernel heap, and unused by BIOS at this point.
 */

#define RING_BASE       0x80000
#define RING_DATA       ((volatile char     *)RING_BASE)
#define RING_WRITE_PTR  ((volatile uint16_t *)(RING_BASE + INPUT_BUFFER_SIZE))
#define RING_READ_PTR   ((volatile uint16_t *)(RING_BASE + INPUT_BUFFER_SIZE + 2))

/* One-time init: zero out the shared region */
void input_init(void)
{
    for (int i = 0; i < INPUT_BUFFER_SIZE; i++)
        RING_DATA[i] = 0;
    *RING_WRITE_PTR = 0;
    *RING_READ_PTR  = 0;
}

/* ── Called from IRQ1 handler ── */
void input_put_char(char c)
{
    uint16_t wr = *RING_WRITE_PTR;
    uint16_t rd = *RING_READ_PTR;
    uint16_t next = (wr + 1) % INPUT_BUFFER_SIZE;

    if (next == rd)    /* buffer full – drop */
        return;

    RING_DATA[wr] = c;
    *RING_WRITE_PTR = next;
}

/* ── Non-blocking read ── */
char getchar_nonblock(void)
{
    uint16_t rd = *RING_READ_PTR;
    uint16_t wr = *RING_WRITE_PTR;

    if (rd == wr)
        return 0;

    char c = RING_DATA[rd];
    *RING_READ_PTR = (rd + 1) % INPUT_BUFFER_SIZE;
    return c;
}

/* ── Blocking read ── */
char getchar(void)
{
    __asm__ volatile("sti");

    while (*RING_READ_PTR == *RING_WRITE_PTR) {
        /* spin – interrupts are enabled so IRQ1 will update RING_WRITE_PTR */
    }

    uint16_t rd = *RING_READ_PTR;
    char c = RING_DATA[rd];
    *RING_READ_PTR = (rd + 1) % INPUT_BUFFER_SIZE;
    return c;
}

/* ── Check if input is waiting ── */
int input_available(void)
{
    return *RING_READ_PTR != *RING_WRITE_PTR;
}

/* ── Read a full line with echo, backspace support ── */
int getline(char *buf, int max_len)
{
    int pos = 0;

    while (pos < max_len) {
        char c = getchar();

        if (c == '\n' || c == '\r') {
            printf("\n");
            break;
        }

        if (c == '\b' || c == 0x7F) {
            if (pos > 0) {
                pos--;
                printf("\b \b");
            }
            continue;
        }

        if (c >= ' ' && c <= '~') {
            buf[pos++] = c;
            printf("%c", c);
        }
    }

    buf[pos] = '\0';
    return pos;
}
