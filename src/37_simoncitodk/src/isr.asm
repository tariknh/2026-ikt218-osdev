global isr0
global isr1
global isr2

global irq0
global irq1
global irq2
global irq3
global irq4
global irq5
global irq6
global irq7
global irq8
global irq9
global irq10
global irq11
global irq12
global irq13
global irq14
global irq15

extern isr_handler
extern irq_handler

section .text

isr0:
    pusha
    push dword 0
    call isr_handler
    add esp, 4
    popa
    iret

isr1:
    pusha
    push dword 1
    call isr_handler
    add esp, 4
    popa
    iret

isr2:
    pusha
    push dword 2
    call isr_handler
    add esp, 4
    popa
    iret

irq0:
    pusha
    push dword 0
    call irq_handler
    add esp, 4
    popa
    iret

irq1:
    pusha
    push dword 1
    call irq_handler
    add esp, 4
    popa
    iret

irq2:
    pusha
    push dword 2
    call irq_handler
    add esp, 4
    popa
    iret

irq3:
    pusha
    push dword 3
    call irq_handler
    add esp, 4
    popa
    iret

irq4:
    pusha
    push dword 4
    call irq_handler
    add esp, 4
    popa
    iret

irq5:
    pusha
    push dword 5
    call irq_handler
    add esp, 4
    popa
    iret

irq6:
    pusha
    push dword 6
    call irq_handler
    add esp, 4
    popa
    iret

irq7:
    pusha
    push dword 7
    call irq_handler
    add esp, 4
    popa
    iret

irq8:
    pusha
    push dword 8
    call irq_handler
    add esp, 4
    popa
    iret

irq9:
    pusha
    push dword 9
    call irq_handler
    add esp, 4
    popa
    iret

irq10:
    pusha
    push dword 10
    call irq_handler
    add esp, 4
    popa
    iret

irq11:
    pusha
    push dword 11
    call irq_handler
    add esp, 4
    popa
    iret

irq12:
    pusha
    push dword 12
    call irq_handler
    add esp, 4
    popa
    iret

irq13:
    pusha
    push dword 13
    call irq_handler
    add esp, 4
    popa
    iret

irq14:
    pusha
    push dword 14
    call irq_handler
    add esp, 4
    popa
    iret

irq15:
    pusha
    push dword 15
    call irq_handler
    add esp, 4
    popa
    iret
