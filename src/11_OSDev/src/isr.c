#include "isr.h"
#include "io.h"
#include "libc/stdio.h"
#include "kernel/pit.h"

static const char *exception_names[32] = {
    "Division Error",
    "Debug",
    "Non-Maskable Interrupt",
    "Breakpoint",
    "Overflow",
    "Bound Range Exceeded",
    "Invalid Opcode",
    "Device Not Available",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Invalid TSS",
    "Segment Not Present",
    "Stack-Segment Fault",
    "General Protection Fault",
    "Page Fault",
    "Reserved",
    "x87 Floating-Point Exception",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating-Point Exception",
    "Virtualization Exception",
    "Control Protection Exception",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Security Exception",
    "Reserved",
};

void isr_handler(struct registers *regs)
{
    printf("ISR %u: %s", regs->int_no, exception_names[regs->int_no]);
    if (regs->err_code)
        printf(" (err=0x%x)", regs->err_code);
    printf("\n");
}

static const char scancode_table[128] = {
/*       0     1     2     3     4     5     6     7  */
/* 00 */  0,    0,   '1',  '2',  '3',  '4',  '5',  '6',
/* 08 */ '7',  '8',  '9',  '0',  '-',  '=',  '\b', '\t',
/* 10 */ 'q',  'w',  'e',  'r',  't',  'y',  'u',  'i',
/* 18 */ 'o',  'p',  '[',  ']',  '\n',  0,   'a',  's',
/* 20 */ 'd',  'f',  'g',  'h',  'j',  'k',  'l',  ';',
/* 28 */ '\'', '`',   0,   '\\', 'z',  'x',  'c',  'v',
/* 30 */ 'b',  'n',  'm',  ',',  '.',  '/',   0,   '*',
/* 38 */  0,   ' ',   0,    0,    0,    0,    0,    0,
/* 40 */  0,    0,    0,    0,    0,    0,    0,   '7',
/* 48 */ '8',  '9',  '-',  '4',  '5',  '6',  '+',  '1',
/* 50 */ '2',  '3',  '0',  '.',   0,    0,    0,    0,
/* 58 */  0,    0,    0,    0,    0,    0,    0,    0,
/* 60 */  0,    0,    0,    0,    0,    0,    0,    0,
/* 68 */  0,    0,    0,    0,    0,    0,    0,    0,
/* 70 */  0,    0,    0,    0,    0,    0,    0,    0,
/* 78 */  0,    0,    0,    0,    0,    0,    0,    0,
};

#define KB_BUFFER_SIZE 256
static char     kb_buffer[KB_BUFFER_SIZE];
static uint32_t kb_head = 0;
static uint32_t kb_tail = 0;

static uint32_t irq_counts[16]  = {0};
static char     last_keypress   = 0;

uint32_t get_irq_count(uint8_t irq)
{
    return (irq < 16) ? irq_counts[irq] : 0;
}

char get_last_keypress(void)
{
    return last_keypress;
}

char kb_getchar(void)
{
    while (kb_tail == kb_head)
        __asm__ volatile("sti\n\t hlt");
    char c = kb_buffer[kb_tail % KB_BUFFER_SIZE];
    kb_tail++;
    return c;
}

char kb_peek(void)
{
    if (kb_tail == kb_head) return 0;
    return kb_buffer[kb_tail % KB_BUFFER_SIZE];
}

char kb_consume(void)
{
    if (kb_tail == kb_head) return 0;
    char c = kb_buffer[kb_tail % KB_BUFFER_SIZE];
    kb_tail++;
    return c;
}

void kb_flush(void)
{
    kb_tail = kb_head;
}

static void keyboard_handler(void)
{
    uint8_t scancode = inb(0x60);
    if (scancode & 0x80) return;   

    char c = scancode_table[scancode & 0x7F];
    if (c == 0) return;

    last_keypress = c;
    kb_buffer[kb_head % KB_BUFFER_SIZE] = c;
    kb_head++;
    printf("%c", c);
}

void irq_handler(struct registers *regs)
{
    
    if (regs->int_no >= 32 && regs->int_no < 48)
        irq_counts[regs->int_no - 32]++;

    switch (regs->int_no) {
        case 32: pit_tick();         break;  
        case 33: keyboard_handler(); break;  
        default: break;
    }

    
    if (regs->int_no >= 40)
        outb(0xA0, 0x20);

    outb(0x20, 0x20);
}
