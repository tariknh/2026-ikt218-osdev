global irq0
global irq1

extern irq0_handler
extern irq1_handler

section .text
bits 32

irq0:
    cli
    pusha
    call irq0_handler
    popa
    sti
    iret

irq1:
    cli
    pusha
    call irq1_handler
    popa
    sti
    iret