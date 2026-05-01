; Make IRQ labels visible outside this file
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

; Use C handlers from another file
extern irq0_handler
extern irq1_handler
extern irq2_handler
extern irq3_handler
extern irq4_handler
extern irq5_handler
extern irq6_handler
extern irq7_handler
extern irq8_handler
extern irq9_handler
extern irq10_handler
extern irq11_handler
extern irq12_handler
extern irq13_handler
extern irq14_handler
extern irq15_handler

; IRQ0 handler
irq0:
    cli
    call irq0_handler
    iret

; IRQ1 handler
irq1:
    cli
    call irq1_handler
    iret

; IRQ2 handler
irq2:
    cli
    call irq2_handler
    iret

; IRQ3 handler
irq3:
    cli
    call irq3_handler
    iret

; IRQ4 handler
irq4:
    cli
    call irq4_handler
    iret

; IRQ5 handler
irq5:
    cli
    call irq5_handler
    iret

; IRQ6 handler
irq6:
    cli
    call irq6_handler
    iret

; IRQ7 handler
irq7:
    cli
    call irq7_handler
    iret

; IRQ8 handler
irq8:
    cli
    call irq8_handler
    iret

; IRQ9 handler
irq9:
    cli
    call irq9_handler
    iret

; IRQ10 handler
irq10:
    cli
    call irq10_handler
    iret

; IRQ11 handler
irq11:
    cli
    call irq11_handler
    iret

; IRQ12 handler
irq12:
    cli
    call irq12_handler
    iret

; IRQ13 handler
irq13:
    cli
    call irq13_handler
    iret

; IRQ14 handler
irq14:
    cli
    call irq14_handler
    iret

; IRQ15 handler
irq15:
    cli
    call irq15_handler
    iret