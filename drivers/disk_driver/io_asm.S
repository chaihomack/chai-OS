bits 32

global inb, outb, inw, outw

inb:
    mov dx, [esp+4]
    in al, dx
    ret

outb:
    mov dx, [esp+4]
    mov al, [esp+8]
    out dx, al
    ret

inw:
    mov dx, [esp+4]
    in ax, dx
    ret

outw:
    mov dx, [esp+4]
    mov ax, [esp+8]
    out dx, ax
    ret