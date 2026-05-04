[bits 32]

global idt_load
extern isr_handler
extern irq_handler

idt_load:
    mov eax, [esp + 4] ; Get IDT pointer argument
    lidt [eax] ; Load the IDT
    ret ; Return to C code

%macro ISR_NOERR 1 
global isr%1
isr%1:
    cli ; Disable interrupts
    push dword 0  ; Push dummy error code
    push dword %1 ; Push interrupt number
    jmp isr_common_stub
%endmacro

%macro ISR_ERR 1
global isr%1
isr%1:
    cli ; Disable interrupts
    push dword %1 ; Push interrupt number
    jmp isr_common_stub
%endmacro

%macro IRQ 2
global irq%1
irq%1:
    cli ; Disable interrupts
    push dword 0 ; Push dummy error code
    push dword %2 ; Push IRQ interrupt number
    jmp irq_common_stub
%endmacro

; CPU exception handlers
ISR_NOERR 0
ISR_NOERR 1
ISR_NOERR 2
ISR_NOERR 3
ISR_NOERR 4
ISR_NOERR 5
ISR_NOERR 6
ISR_NOERR 7
ISR_ERR   8
ISR_NOERR 9
ISR_ERR   10
ISR_ERR   11
ISR_ERR   12
ISR_ERR   13
ISR_ERR   14
ISR_NOERR 15
ISR_NOERR 16
ISR_ERR   17
ISR_NOERR 18
ISR_NOERR 19
ISR_NOERR 20
ISR_NOERR 21
ISR_NOERR 22
ISR_NOERR 23
ISR_NOERR 24
ISR_NOERR 25
ISR_NOERR 26
ISR_NOERR 27
ISR_NOERR 28
ISR_NOERR 29
ISR_ERR   30
ISR_NOERR 31

; Hardware IRQ handlers
IRQ 0, 32
IRQ 1, 33
IRQ 2, 34
IRQ 3, 35
IRQ 4, 36
IRQ 5, 37
IRQ 6, 38
IRQ 7, 39
IRQ 8, 40
IRQ 9, 41
IRQ 10, 42
IRQ 11, 43
IRQ 12, 44
IRQ 13, 45
IRQ 14, 46
IRQ 15, 47

isr_common_stub:
    pusha ; Save general-purpose registers
    mov ax, ds ; Save old data segment
    push eax
    mov ax, 0x10   ; Load kernel data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    push esp  ; Pass register state to C handler
    call isr_handler ; Call C ISR handler
    add esp, 4 ; Remove argument from stack
    pop eax ; Restore old data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    popa ; Restore registers
    add esp, 8 ; Remove interrupt number and error code
    sti ; Enable interrupts
    iretd  ; Return from interrupt

irq_common_stub:
    pusha ; Save general-purpose registers
    mov ax, ds; Save old data segment
    push eax
    mov ax, 0x10  ; Load kernel data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    push esp ; Pass register state to C handler
    call irq_handler; Call C IRQ handler
    add esp, 4  ; Remove argument from stack
    pop eax ; Restore old data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    popa ; Restore registers
    add esp, 8 ; Remove IRQ number and error code
    sti; Enable interrupts
    iretd ; Return from interrupt