global isr0
global isr1
global isr2

extern isr0_handler
extern isr1_handler
extern isr2_handler

section .text
bits 32

isr0:
    cli
    pusha
    call isr0_handler
    popa
    sti
    iret

isr1:
    cli
    pusha
    call isr1_handler
    popa
    sti
    iret

isr2:
    cli
    pusha
    call isr2_handler
    popa
    sti
    iret