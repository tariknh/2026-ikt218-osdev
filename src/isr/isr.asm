global isr0
global isr1
global isr14

extern isr0_handler
extern isr1_handler
extern isr14_handler

section .text
bits 32

isr0:
	pusha
	cld
	call isr0_handler
	popa
	iretd

isr1:
	pusha
	cld
	call isr1_handler
	popa
	iretd

isr14:
	pusha
	cld
	call isr14_handler
	popa
	iretd
