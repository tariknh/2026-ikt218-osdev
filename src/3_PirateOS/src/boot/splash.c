#include "boot/splash.h"

#include "kernel/pit.h"
#include "libc/stdio.h"

// Boot-time splash output belongs with startup presentation rather than the
// terminal driver. It still renders through printf and the VGA-backed terminal.
void terminal_print_logo(void)
{
    uint8_t original_color = terminal_get_color();

    terminal_set_color(0x08);
    printf("                           .  o  .                                   \n");
    terminal_set_color(0x0F);
    printf("                         .^^^^^^^.                                   \n");
    terminal_set_color(0x08);
    printf("                         | X   X |                                   \n");
    printf("                         |   ^   |                                   \n");
    printf("                         | X   X |                                   \n");
    terminal_set_color(0x07);
    printf("                         '-------'                                   \n");
    terminal_set_color(0x06);
    printf("                             |                                       \n");
    printf("                             |                                       \n");
    terminal_set_color(0x0F);
    printf("              .______________|______________.                         \n");
    terminal_set_color(0x07);
    printf("              |  ~  ~  ~  ~  |  ~  ~  ~  ~ |                        \n");
    printf("              |  ~  ~  ~  ~  |  ~  ~  ~  ~ |                        \n");
    terminal_set_color(0x0F);
    printf("              '______________________________|                        \n");
    terminal_set_color(0x06);
    printf("                             |                                       \n");
    terminal_set_color(0x0C);
    printf("          >>=====[ ");
    terminal_set_color(0x0E);
    printf("  P I R A T E   O S  ");
    terminal_set_color(0x0C);
    printf("]=====>                              \n");
    terminal_set_color(0x06);
    printf("                             |                                       \n");
    terminal_set_color(0x0F);
    printf("       .======================|========================.              \n");
    printf("       |    ");
    terminal_set_color(0x07);
    printf("*");
    terminal_set_color(0x0F);
    printf("                                    ");
    terminal_set_color(0x07);
    printf("*");
    terminal_set_color(0x0F);
    printf("    |              \n");
    printf("       |                                                |              \n");
    terminal_set_color(0x08);
    printf("        \\______________________________________________/               \n");
    terminal_set_color(0x0F);
    printf("          \\____________________________________________/                \n");
    terminal_set_color(0x0B);
    printf("  ~~~  ~~~  ~~~  ~~~  ~~~  ~~~  ~~~  ~~~  ~~~  ~~~  ~~~  ~~~  ~~~   \n");
    terminal_set_color(0x09);
    printf("~  ~~~  ~~~  ~~~  ~~~  ~~~  ~~~  ~~~  ~~~  ~~~  ~~~  ~~~  ~~~  ~~~  \n");
    terminal_set_color(0x0E);
    printf("======================================================================\n");
    terminal_set_color(original_color);

    sleep_interrupt(3000);
    terminal_initialize();
}
