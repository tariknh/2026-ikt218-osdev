global isr0, isr1, isr2, isr3, isr4, isr5, isr6, isr7
global isr8, isr9, isr10, isr11, isr12, isr13, isr14, isr15
global isr16, isr17, isr18, isr19, isr20, isr21, isr22, isr23
global isr24, isr25, isr26, isr27, isr28, isr29, isr30, isr31
global irq0, irq1, irq2, irq3, irq4, irq5, irq6, irq7
global irq8, irq9, irq10, irq11, irq12, irq13, irq14, irq15

extern isr_handler
extern irq_handler

;for exceptions that dont auto push error code
%macro isr_no_err_stub 1
isr%1:
    pusha
    push dword %1       ;pass interrupt number to isr_handler
    call isr_handler
    add esp, 4         ;clean up pushed argument
    popa
    iret
%endmacro

;for exceptions that auto push error code
%macro isr_err_stub 1
isr%1:
    pusha
    push dword %1      ;pass interrupt number to isr_handler
    call isr_handler
    add esp, 4        ;clean up pushed argument
    popa
    add esp, 4       ;discard error code pushed by CPU
    iret
%endmacro

%macro irq_no_err_stub 1
irq%1:
    pusha
    push dword %1       ;pass IRQ number to irq_handler
    call irq_handler
    add esp, 4         
    popa
    iret
%endmacro

isr_no_err_stub 0
isr_no_err_stub 1
isr_no_err_stub 2
isr_no_err_stub 3
isr_no_err_stub 4
isr_no_err_stub 5
isr_no_err_stub 6
isr_no_err_stub 7
isr_err_stub 8
isr_no_err_stub 9
isr_err_stub 10
isr_err_stub 11
isr_err_stub 12
isr_err_stub 13
isr_err_stub 14
isr_no_err_stub 15
isr_no_err_stub 16
isr_err_stub 17
isr_no_err_stub 18
isr_no_err_stub 19
isr_no_err_stub 20
isr_no_err_stub 21
isr_no_err_stub 22
isr_no_err_stub 23
isr_no_err_stub 24
isr_no_err_stub 25
isr_no_err_stub 26
isr_no_err_stub 27
isr_no_err_stub 28
isr_no_err_stub 29
isr_err_stub 30
isr_no_err_stub 31

irq_no_err_stub 0
irq_no_err_stub 1
irq_no_err_stub 2
irq_no_err_stub 3
irq_no_err_stub 4
irq_no_err_stub 5
irq_no_err_stub 6
irq_no_err_stub 7
irq_no_err_stub 8
irq_no_err_stub 9
irq_no_err_stub 10
irq_no_err_stub 11
irq_no_err_stub 12
irq_no_err_stub 13
irq_no_err_stub 14
irq_no_err_stub 15