; isr.asm - 32 ISR entry points for CPU exceptions
;
; The CPU automatically pushes EFLAGS, CS and EIP when an interrupt fires.
; For some exceptions it also pushes a 4-byte error code; for the rest it
; doesn't.  Either way, our common handler expects a uniform stack frame,
; so the stubs that don't get a real error code push a dummy zero first.
;
; After the stub runs, the stack contains:
;
;   [esp + 0]   int_no       (pushed by stub)
;   [esp + 4]   err_code     (pushed by CPU or as dummy by stub)
;   [esp + 8]   eip          (pushed by CPU)
;   [esp + 12]  cs
;   [esp + 16]  eflags
;   ...

%macro ISR_NOERR 1
global isr%1
isr%1:
    cli
    push    dword 0           ; dummy error code
    push    dword %1          ; vector number
    jmp     isr_common
%endmacro

%macro ISR_ERR 1
global isr%1
isr%1:
    cli
    push    dword %1          ; vector number (CPU already pushed err_code)
    jmp     isr_common
%endmacro

extern isr_handler

section .text
bits 32

; According to the Intel manual, vectors 8, 10, 11, 12, 13, 14 and 17 push
; an error code automatically. The rest don't.
ISR_NOERR 0          ; division by zero
ISR_NOERR 1          ; debug
ISR_NOERR 2          ; non-maskable interrupt
ISR_NOERR 3          ; breakpoint (used by INT 3)
ISR_NOERR 4          ; into detected overflow
ISR_NOERR 5          ; out of bounds
ISR_NOERR 6          ; invalid opcode
ISR_NOERR 7          ; no coprocessor
ISR_ERR   8          ; double fault
ISR_NOERR 9          ; coprocessor segment overrun
ISR_ERR   10         ; bad tss
ISR_ERR   11         ; segment not present
ISR_ERR   12         ; stack fault
ISR_ERR   13         ; general protection fault
ISR_ERR   14         ; page fault
ISR_NOERR 15         ; reserved
ISR_NOERR 16         ; coprocessor fault
ISR_ERR   17         ; alignment check
ISR_NOERR 18         ; machine check
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
ISR_NOERR 30
ISR_NOERR 31

; Common stub: save full register state, switch to kernel data segment,
; call the C handler, then restore everything and return with iret.
isr_common:
    pusha                       ; pushes EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI

    mov     ax, ds              ; save current data segment
    push    eax

    mov     ax, 0x10            ; kernel data selector (our GDT entry 2)
    mov     ds, ax
    mov     es, ax
    mov     fs, ax
    mov     gs, ax

    push    esp                 ; pointer to registers struct (= current esp)
    call    isr_handler
    add     esp, 4              ; remove the pushed esp argument

    pop     eax                 ; restore the original data segment
    mov     ds, ax
    mov     es, ax
    mov     fs, ax
    mov     gs, ax

    popa
    add     esp, 8              ; remove int_no and err_code
    iret
