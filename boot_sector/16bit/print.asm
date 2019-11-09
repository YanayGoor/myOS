print:
    ;bx is the address of the string
    pusha
    mov ah, 0x0e

    myloop:
        mov al, [bx]
        cmp al, 0
        je endloop
        int 0x10
        inc bx
        jmp myloop
    endloop:
        popa
        ret

print_ln:
    ;bx is the address of the string
    pusha
    mov ah, 0x0e
    mov al, 0x0a
    int 0x10
    mov al, 0x0d
    int 0x10
    popa
    ret
