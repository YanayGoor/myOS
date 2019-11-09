printhex_from_address:
    pusha
    ;bx is the address of the bytes
    ;cx is the length to print

    startloop:
        mov al, [bx]

        call printhex

        inc bx
        loop startloop
    popa
    ret

printhex:
    pusha
    ;al is the print
    mov bl, al

    mov al, bl

    shr al, 4

    call print_hex_digit

    mov al, bl

    and al, 0x0f

    call print_hex_digit
    popa
    ret


print_hex_digit:
    pusha
    mov ah, 0x0e
    cmp al, 9
    jg addchar2
        add al, 0x30
        jmp endif2
    addchar2:
        add al, 0x37
    endif2:
        int 0x10 ; prints A
        popa
        ret