extern isr_handler

global isr0
global isr6
global isr13

%macro ISR_NOERR 1
isr%1:
    cli                   ; Clear interrupts
    push dword 0          ; Push a dummy error code (0) onto the stack
    push dword %1         ; Push the interrupt number onto the stack
    jmp isr_common_stub ; Jump to the common ISR handler
%endmacro

%macro ISR_ERR 1
isr%1:
    cli                   ; Clear interrupts
    push dword %1         ; Push the interrupt number onto the stack
    jmp isr_common_stub ; Jump to the common ISR handler
%endmacro

isr_common_stub:
    pusha                 ; Push all general-purpose registers onto the stack

    mov ax, ds
    push eax                 ; Push the data segment selector onto the stack

    mov ax, 0x10            ; Load the kernel data segment selector (0x10) into ax
    mov ds, ax              ; Set the data segment register to the kernel data segment
    mov es, ax              ; Set the extra segment register to the kernel data segment
    mov fs, ax              ; Set the fs segment register to the kernel data segment
    mov gs, ax              ; Set the gs segment register to the kernel data segment

    push esp                 ; Push the current stack pointer onto the stack
    call isr_handler         ; Call the common ISR handler function
    add esp, 4              ; Clean up the stack after the call

    pop eax                  ; Restore the original data segment selector from the stack
    mov ds, ax              ; Restore the original data segment register
    mov es, ax              ; Restore the original extra segment register
    mov fs, ax              ; Restore the original fs segment register
    mov gs, ax              ; Restore the original gs segment register

    popa                   ; Restore all general-purpose registers from the stack
    add esp, 8              ; Clean up the stack (remove the error code and interrupt number)
    sti                   ; Set interrupts
    iret                  ; Return from the interrupt

ISR_NOERR 0   ; Divide by zero exception
ISR_NOERR 6   ; Invalid opcode exception
ISR_ERR 13    ; General protection fault