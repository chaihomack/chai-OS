bits 32

section .text
global kstart
global keyboard_handler
global read_port
global write_port
global load_idt

extern kmain 		;this is defined in the c file
extern keyboard_handler_main

kstart:
	cli 				;block interrupts
	mov esp, stack_space
	call kmain
	
	jmp $

load_idt:
	mov edx, [esp + 4]
	lidt [edx]
	sti 				;turn on interrupts
	ret

keyboard_handler:                 
	call    keyboard_handler_main
	iretd


section .bss
resb 8192; 8KB for stack
stack_space: