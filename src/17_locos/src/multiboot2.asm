; Name: multiboot2.asm
; Project: LocOS
; Description: This file is the bridge between the bootloader and kernel.c.
; It declares a multiboot header so that the bootloader knows this kernel supports Multiboot2
; and is bootable in the expected way.


extern kmain ; Says that kmain is defined in a nother file which is in kernel.c

global _start ; Export the _start symbol so the linker can find it

section .multiboot_header ; Starts the Multiboot2 header section. The bootloader scans for this
header_start: ; Header beginning
    dd 0xe85250d6 	                                                ; Magic number (multiboot 2)
    dd 0				                                            ; Architecture 0 (protected mode i386)
    dd header_end - header_start 	                                ; Header length
    dd 0x100000000 - (0xe85250d6 + 0 + (header_end - header_start)) ; Checksum

;align 8
;framebuffer_tag_start:
;    dw 5                                              ; type
;    dw 1                                              ; flags
;    dd framebuffer_tag_end - framebuffer_tag_start    ; size
;    dd 800                                            ; width
;    dd 600                                            ; height
;    dd 32                                             ; depth
;framebuffer_tag_end:

align 8 ; Aligns next data on 8 byte boundary which is required by the multiboot2 specification for tags
  
    dw 0	; type
    dw 0	; flags
    dw 8	; size
header_end:

section .text ; Starts the code section
bits 32 ; Tells assembler this is 32 bit code

_start: ; Label for the first executed instruction
    cli ; Clear interrupt flag disabling maskable interrupts

    mov esp, stack_top ; Sets the stack pointer to top of reserved stack space

	push ebx ; pushes multiboot structure pointer
	push eax ; pushes magic number

    call kmain ; Jump main function

section .bss ; Starts the uninitialized data section
stack_bottom: ; Label for the bottom of the stack
    resb 4096 * 16 ;reserves 64 kilo bytes for stack space
stack_top: ;label at the end of stack space