bits 32

section .text
extern kernel_load

global sec_stage_start
sec_stage_start:
    cli
    mov esp, stack_space

    call kernel_load

    cli 
    hlt


section .bss
resb 8192; 8KB for stack
stack_space: