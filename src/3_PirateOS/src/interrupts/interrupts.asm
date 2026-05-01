; Interrupt handler stubs

%macro ISR_NOERRCODE 1 ; ISR without error code from the CPU
  global isr%1  ; Declare the ISR as global so it can be referenced from other files
  isr%1: ; Define the ISR label
    cli ; Clear interrupts to prevent nested interrupts
    push byte 0 ; Push a dummy error code (0) onto the stack for consistency
    push byte %1 ; Push the ISR number onto the stack
    jmp isr_common_stub ; Jump to the common ISR handler stub
%endmacro ; End of the ISR_NOERRCODE macro definition

%macro ISR_ERRCODE 1 ; ISR with error code from the CPU
  global isr%1 ; Declare the ISR as global so it can be referenced from other files
  isr%1: ; Define the ISR label
    cli ; Clear interrupts to prevent nested interrupts
    push byte %1 ; Push the ISR number onto the stack
    jmp isr_common_stub ; Jump to the common ISR handler stub
%endmacro

%macro IRQ 2 ; IRQ handler stub
  global irq%1 ; Declare the IRQ handler as global so it can be referenced from other files
  irq%1: ; Define the IRQ handler label
    cli ; Clear interrupts to prevent nested interrupts
    push byte 0 ; Push a dummy error code (0) onto the stack for consistency
    push byte %2 ; Push the IRQ number (offset by 32) onto the stack
    jmp irq_common_stub ; Jump to the common IRQ handler stub
%endmacro

; Define ISRs for CPU exceptions (0-31)
ISR_NOERRCODE 0
ISR_NOERRCODE 1
ISR_NOERRCODE 2
ISR_NOERRCODE 3
ISR_NOERRCODE 4
ISR_NOERRCODE 5
ISR_NOERRCODE 6
ISR_NOERRCODE 7
ISR_ERRCODE   8
ISR_NOERRCODE 9
ISR_ERRCODE   10
ISR_ERRCODE   11
ISR_ERRCODE   12
ISR_ERRCODE   13
ISR_ERRCODE   14
ISR_NOERRCODE 15
ISR_NOERRCODE 16
ISR_NOERRCODE 17
ISR_NOERRCODE 18
ISR_NOERRCODE 19
ISR_NOERRCODE 20
ISR_NOERRCODE 21
ISR_NOERRCODE 22
ISR_NOERRCODE 23
ISR_NOERRCODE 24
ISR_NOERRCODE 25
ISR_NOERRCODE 26
ISR_NOERRCODE 27
ISR_NOERRCODE 28
ISR_NOERRCODE 29
ISR_NOERRCODE 30
ISR_NOERRCODE 31

; Define IRQ handlers for hardware interrupts (32-47)
IRQ   0,    32
IRQ   1,    33
IRQ   2,    34
IRQ   3,    35
IRQ   4,    36
IRQ   5,    37
IRQ   6,    38
IRQ   7,    39
IRQ   8,    40
IRQ   9,    41
IRQ  10,    42
IRQ  11,    43
IRQ  12,    44
IRQ  13,    45
IRQ  14,    46
IRQ  15,    47

; Common ISR and IRQ handler stubs
extern isr_handler
extern irq_handler


isr_common_stub: ; Common ISR handler stub
    pusha ; Push all general-purpose registers onto the stack to preserve their state
    
    mov ax, ds ; Save the current data segment selector
    push eax ; Push it onto the stack to restore later
    
    mov ax, 0x10  ; load the kernel data segment descriptor
    mov ds, ax ; Set the data segment to the kernel data segment
    mov es, ax ; Set the extra segment to the kernel data segment
    mov fs, ax ; Set the FS segment to the kernel data segment
    mov gs, ax ; Set the GS segment to the kernel data segment
    
    push esp ; Push the current stack pointer (ESP) onto the stack to pass it to the handler
    call isr_handler ; Call the common ISR handler function defined in C
    pop eax  ; Discard the esp argument pushed before the call
    
    pop eax ; Restore the original data segment selector from the stack
    mov ds, ax ; Restore the original data segment
    mov es, ax ; Restore the original extra segment
    mov fs, ax ; Restore the original FS segment
    mov gs, ax ; Restore the original GS segment
    
    popa ; Restore all general-purpose registers from the stack
    add esp, 8     ; Cleans up the pushed error code and pushed ISR number
    sti ; Set interrupts to allow other interrupts to be handled
    iret ; Return from the interrupt, restoring the instruction pointer and flags

irq_common_stub: ; Common IRQ handler stub
    pusha  ; Push all general-purpose registers onto the stack to preserve their state

    mov ax, ds ; Save the current data segment selector
    push eax ; Push it onto the stack to restore later

    mov ax, 0x10 ; load the kernel data segment descriptor
    mov ds, ax ; Set the data segment to the kernel data segment
    mov es, ax ; Set the extra segment to the kernel data segment
    mov fs, ax ; Set the FS segment to the kernel data segment
    mov gs, ax ; Set the GS segment to the kernel data segment

    push esp ; Push the current stack pointer (ESP) onto the stack to pass it to the handler
    call irq_handler ; Call the common IRQ handler function defined in C
    pop eax  ; Discard the esp argument pushed before the call
    pop ebx ; Restore the original data segment selector from the stack

    ;pop ebx ; Restore the original data segment selector from the stack
    mov ds, bx ; Restore the original data segment
    mov es, bx ; Restore the original extra segment
    mov fs, bx ; Restore the original FS segment
    mov gs, bx ; Restore the original GS segment

    popa ; Restore all general-purpose registers from the stack
    add esp, 8 ; Cleans up the pushed error code and pushed IRQ number
    sti ; Set interrupts to allow other interrupts to be handled
    iret ; Return from the interrupt, restoring the instruction pointer and flags
