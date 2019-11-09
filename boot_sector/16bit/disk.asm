DiskAddressPacket:          db 16,0
.SectorsToRead:             dw 54                        ; Number of sectors to read (read size of OS)
.Offset:                    dw 0x1000                              ; Offset :0000
.Segment:                   dw 0x0000                          ; Segment 0200
.End:                       dq 1                             ; Sector 16 or 10h on CD-ROM

read_from_disk:
    ; dh: number of sectors to read
    pusha

    push dx
    MOV SI,DiskAddressPacket
    mov ah, 0x42
;    mov al, dh
;    mov cl, 2
;    mov ch, 0
;    mov dh, 0
;    mov dl, 0x80
    int 0x13
    jc print_error

    ; else

    pop dx

;    cmp al, 16
;    jne print_sector_error

return_from_disk:
    popa
    ret


print_error:
    mov bx, DISK_ERROR
    call print
    call print_ln
    ; ah = error code, dl = disk drive that dropped the error
    mov al, ah
    call printhex
    mov al, dh
    call printhex
    jmp return_from_disk

print_sector_error:
    mov bx, SECTORS_ERROR
    call print
    call print_ln
    ; ah = error code, dl = disk drive that dropped the error
;    mov al, ah
    call printhex
    mov al, dh
    call printhex
    jmp return_from_disk



DISK_ERROR: db "Disk read error", 0
SECTORS_ERROR: db "Incorrect number of sectors read", 0