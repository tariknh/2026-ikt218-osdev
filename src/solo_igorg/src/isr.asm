global isr0
global isr3
global isr4

extern isr0_handler
extern isr3_handler
extern isr4_handler

; These ISR stubs save the general-purpose registers, call the matching C handler, 
; restore the registers and return from interrupt with iret.

isr0:
    pusha
    call isr0_handler
    popa
    iret

isr3:
    pusha
    call isr3_handler
    popa
    iret

isr4:
    pusha
    call isr4_handler
    popa
    iret
