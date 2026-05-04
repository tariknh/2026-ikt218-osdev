
global idt_flush

; ISR stubs — CPU exceptions (vectors 0-31)
global isr0,  isr1,  isr2,  isr3,  isr4,  isr5,  isr6,  isr7
global isr8,  isr9,  isr10, isr11, isr12, isr13, isr14, isr15
global isr16, isr17, isr18, isr19, isr20, isr21, isr22, isr23
global isr24, isr25, isr26, isr27, isr28, isr29, isr30, isr31

; IRQ stubs — hardware interrupts (remapped to vectors 32-47)
global irq0,  irq1,  irq2,  irq3,  irq4,  irq5,  irq6,  irq7
global irq8,  irq9,  irq10, irq11, irq12, irq13, irq14, irq15

extern isr_handler   ; defined in isr.c
extern irq_handler   ; defined in isr.c

section .text
bits 32

idt_flush:
    mov  eax, [esp+4]   ; address of our idt_descriptor struct
    lidt [eax]          ; load IDT register
    sti                 ; enable hardware interrupts
    ret




%macro ISR_NOERRCODE 1
isr%1:
    push dword 0        ; dummy error code
    push dword %1       ; interrupt vector number
    jmp  isr_common_stub
%endmacro


%macro ISR_ERRCODE 1
isr%1:
    push dword %1       ; interrupt vector number
    jmp  isr_common_stub
%endmacro

;  CPU Exception Stubs 
ISR_NOERRCODE  0    ; #DE  Divide Error
ISR_NOERRCODE  1    ; #DB  Debug
ISR_NOERRCODE  2    ;      Non-Maskable Interrupt
ISR_NOERRCODE  3    ; #BP  Breakpoint
ISR_NOERRCODE  4    ; #OF  Overflow
ISR_NOERRCODE  5    ; #BR  Bound Range Exceeded
ISR_NOERRCODE  6    ; #UD  Invalid Opcode
ISR_NOERRCODE  7    ; #NM  Device Not Available
ISR_ERRCODE    8    ; #DF  Double Fault              (error code = 0)
ISR_NOERRCODE  9    ;      Coprocessor Segment Overrun (obsolete)
ISR_ERRCODE   10    ; #TS  Invalid TSS
ISR_ERRCODE   11    ; #NP  Segment Not Present
ISR_ERRCODE   12    ; #SS  Stack-Segment Fault
ISR_ERRCODE   13    ; #GP  General Protection Fault
ISR_ERRCODE   14    ; #PF  Page Fault
ISR_NOERRCODE 15    ;      Reserved
ISR_NOERRCODE 16    ; #MF  x87 Floating-Point Exception
ISR_ERRCODE   17    ; #AC  Alignment Check
ISR_NOERRCODE 18    ; #MC  Machine Check
ISR_NOERRCODE 19    ; #XM  SIMD Floating-Point Exception
ISR_NOERRCODE 20    ; #VE  Virtualization Exception
ISR_ERRCODE   21    ; #CP  Control Protection Exception
ISR_NOERRCODE 22    ;      Reserved
ISR_NOERRCODE 23    ;      Reserved
ISR_NOERRCODE 24    ;      Reserved
ISR_NOERRCODE 25    ;      Reserved
ISR_NOERRCODE 26    ;      Reserved
ISR_NOERRCODE 27    ;      Reserved
ISR_NOERRCODE 28    ;      Reserved
ISR_NOERRCODE 29    ;      Reserved
ISR_ERRCODE   30    ; #SX  Security Exception
ISR_NOERRCODE 31    ;      Reserved



%macro IRQ 2
irq%1:
    push dword 0        ; dummy error code (IRQs never carry one)
    push dword %2       ; vector number (32-47)
    jmp  irq_common_stub
%endmacro

;  Hardware IRQ Stubs 
IRQ  0, 32    ; Timer
IRQ  1, 33    ; PS/2 Keyboard
IRQ  2, 34    ; Cascade (internal, not raised to CPU)
IRQ  3, 35    ; COM2
IRQ  4, 36    ; COM1
IRQ  5, 37    ; LPT2 / Sound card
IRQ  6, 38    ; Floppy disk
IRQ  7, 39    ; LPT1 / Spurious
IRQ  8, 40    ; CMOS Real-Time Clock
IRQ  9, 41    ; Free / ACPI / Legacy NIC
IRQ 10, 42    ; Free / SCSI / NIC
IRQ 11, 43    ; Free / SCSI / NIC
IRQ 12, 44    ; PS/2 Mouse
IRQ 13, 45    ; FPU / Coprocessor / Inter-processor
IRQ 14, 46    ; Primary ATA
IRQ 15, 47    ; Secondary ATA



isr_common_stub:
    pusha                   ; push EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI

    mov  ax, ds
    push eax                ; save current data segment (as full 32-bit word)

    mov  ax, 0x10           ; 0x10 = kernel data segment selector
    mov  ds, ax
    mov  es, ax
    mov  fs, ax
    mov  gs, ax

    push esp                ; argument: pointer to saved registers struct
    call isr_handler        ; call C handler
    add  esp, 4             ; discard the pointer argument

    pop  eax                ; restore saved data segment
    mov  ds, ax
    mov  es, ax
    mov  fs, ax
    mov  gs, ax

    popa                    ; restore all general-purpose registers
    add  esp, 8             ; discard int_no and err_code
    iret                    ; restore EIP, CS, EFLAGS; resume interrupted code



irq_common_stub:
    pusha

    mov  ax, ds
    push eax

    mov  ax, 0x10
    mov  ds, ax
    mov  es, ax
    mov  fs, ax
    mov  gs, ax

    push esp
    call irq_handler
    add  esp, 4

    pop  eax
    mov  ds, ax
    mov  es, ax
    mov  fs, ax
    mov  gs, ax

    popa
    add  esp, 8
    iret
