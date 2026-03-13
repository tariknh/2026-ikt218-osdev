bits 32

global isr3
global isr4
global isr5

extern isr_handler

%macro ISR_NOERR 1
isr%1:
    pushad
    push dword %1
    call isr_handler
    add esp, 4
    popad
    iretd
%endmacro

ISR_NOERR 3
ISR_NOERR 4
ISR_NOERR 5
