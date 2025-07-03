bits 32

section .text
extern kernel_load

global sec_stage_start
sec_stage_start:
    cli
    mov esp, stack_space

    mov edi, 0xB8002    ; vid pointer
    mov al, 'i'         
    mov ah, 0x0F        ; white on black
    mov [edi], ax       ; load to vid memory

    call kernel_load

    cli 
    hlt


section .bss
resb 8192; 8KB for stack
stack_space: