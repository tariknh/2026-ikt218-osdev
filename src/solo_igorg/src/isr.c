#include <isr.h>
#include <terminal.h>

void isr0_handler(void)
{
    terminal_write("ISR 0x00 triggered: divide-by-zero vector\n");
}

void isr3_handler(void)
{
    terminal_write("ISR 0x03 triggered: breakpoint vector\n");
}

void isr4_handler(void)
{
    terminal_write("ISR 0x04 triggered: overflow vector\n");
}
