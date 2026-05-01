[BITS 32]
; This file contains the assembly code for setting up the Interrupt Descriptor Table (IDT) 
;and defining the common stubs for handling interrupts and exceptions. The IDT is a critical data structure that allows the CPU to 
;know how to respond to various interrupts, whether they are hardware interrupts from devices or software exceptions.
; Define the loader function called from idt.c
global idt_load
extern idtp
idt_load:
    lidt [idtp]
    ret

; Common ISR stub: This saves the processor state before calling C
extern isr_handler
isr_common_stub:
    pusha                    ; Pushes edi,esi,ebp,esp,ebx,edx,ecx,eax
    mov ax, ds               ; Lower 16-bits of eax = ds
    push eax                 ; save the data segment descriptor

    mov ax, 0x10             ; load the kernel data segment descriptor
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push esp 
    call isr_handler         ; Call the C-level handler!
    add esp, 4               ; Clean up the pushed esp

    pop eax                  ; reload the original data segment descriptor
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    popa                     ; Pops edi,esi,ebp...
    add esp, 8               ; Cleans up the pushed error code and ISR number
    iret                     ; pops 5 things at once: CS, EIP, EFLAGS, SS, and ESP

; Macros to create ISRs easily
%macro ISR_NOERRCODE 1
  global isr%1
  isr%1:
    push byte 0              ; Dummy error code
    push byte %1             ; Interrupt number
    jmp isr_common_stub
%endmacro

; Define the stubs for the ISRs 
ISR_NOERRCODE 0              ; Divide by zero
ISR_NOERRCODE 1              ; Debug
ISR_NOERRCODE 2              ; Non-maskable interrupt

extern irq_handler

irq_common_stub:
    pusha
    mov ax, ds
    push eax

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push esp
    call irq_handler
    add esp, 4

    pop eax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    popa
    add esp, 8
    iret

%macro IRQ 2
  global irq%1
  irq%1:
    push dword 0
    push dword %2
    jmp irq_common_stub
%endmacro

; Define the stubs for the IRQs
IRQ 0, 32   ; Timer
IRQ 1, 33   ; Keyboard (Task 4)
IRQ 2, 34   ; Cascade
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