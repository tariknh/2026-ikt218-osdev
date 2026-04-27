[bits 32]

global switch_to_user_mode

section .text

switch_to_user_mode:
    MOV   EDX, [ESP + 4]   ; entry
    MOV   ECX, [ESP + 8]  ; stack_top

    PUSH  0x23  ; user SS
    PUSH  ECX   ; user ESP
    PUSH  0x202 ; EFLAGS
    PUSH  0x1B  ; user CS
    PUSH  EDX   ; user EIP
    
    IRETD
