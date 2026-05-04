bits 32

global irq0_stub
global irq1_stub

extern irq0_handler
extern irq1_handler

section .text

irq0_stub:
    pusha
    call irq0_handler
    popa
    iretd

irq1_stub:
    pusha
    call irq1_handler
    popa
    iretd