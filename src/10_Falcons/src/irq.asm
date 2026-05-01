extern irq_handler

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

%macro IRQ_STUB 2
irq%1:
    cli
    push dword %2
    call irq_handler
    add esp, 4
    sti
    iret
%endmacro

IRQ_STUB 0, 0
IRQ_STUB 1, 1
IRQ_STUB 2, 2
IRQ_STUB 3, 3
IRQ_STUB 4, 4
IRQ_STUB 5, 5
IRQ_STUB 6, 6
IRQ_STUB 7, 7
IRQ_STUB 8, 8
IRQ_STUB 9, 9
IRQ_STUB 10, 10
IRQ_STUB 11, 11
IRQ_STUB 12, 12
IRQ_STUB 13, 13
IRQ_STUB 14, 14
IRQ_STUB 15, 15