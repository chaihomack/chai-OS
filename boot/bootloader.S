[org 0x7C00]
[bits 16]

start:
    xor ax, ax
    mov ds, ax          ; everything needs to be 0
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00  

    call load_sec_stage_in_mem

    ;A20
    in al, 0x92
    or al, 2
    out 0x92, al

    ;prepearing for 32 bits mode
    cli                         
    lgdt [gdt_descriptor]  
    mov eax, cr0
    or eax, 0x1    
    mov cr0, eax
    jmp CODE_SEG:init_pm  ; lets 32 bits
    
load_sec_stage_in_mem:
    mov ax, 0x900
    mov es, ax          ; es:bx adressing so 0x900:0x0000 = 0x9000
    xor bx, bx

    mov ah, 0x02
    mov al, 20          ; loading first 20 sectors to memory(might be full second stage)
    mov ch, 0x00
    mov cl, 2
    mov dh, 0x00
    mov dl, 0x80
    int 0x13            ;this interrupt uses ex:bx adressing
    ret

[bits 32]

init_pm:
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax              ; everything needs to be 0
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov ebp, 0x90000
    mov esp, ebp
    
    jmp 0x9000
    cli
    hlt

gdt_start:
    gdt_null:
        dd 0
        dd 0
    gdt_code:
        dw 0xFFFF
        dw 0
        db 0
        db 10011010b
        db 11001111b
        db 0
    gdt_data:
        dw 0xFFFF
        dw 0
        db 0
        db 10010010b
        db 11001111b
        db 0
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1  ; gdt size
    dd gdt_start                ; gdt adress

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start


times 510-($-$$) db 0
dw 0xAA55