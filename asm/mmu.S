bits 32

global mmu_setup

mmu_setup:
    mov edx, [esp + 4]  ; first arg, address of pdt
    
    mov cr3, edx    ; loading address of pdt 
    mov eax, cr0    ; saving current bits
    or eax, (1<<31) ; placing bit to turn on paging
    mov cr0, eax    ; turning on paging
    ret