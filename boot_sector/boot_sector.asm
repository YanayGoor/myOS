[org 0x7c00]
KERNEL_OFFSET equ 0x1000 ; The same one we used when linking the kernel

    mov [BOOT_DRIVE], dl ; Remember that the BIOS sets us the boot drive in 'dl' on boot
    mov bp, 0x9000
    mov sp, bp

    mov bx, MSG_REAL_MODE
    call print
    call print_ln

    call load_kernel ; read the kernel from disk
    call switch_to_pm ; disable interrupts, load GDT,  etc. Finally jumps to 'BEGIN_PM'
    jmp $ ; Never executed


%include "boot_sector/16bit/print.asm"
%include "boot_sector/16bit/print_hex.asm"
%include "boot_sector/16bit/disk.asm"
%include "boot_sector/32bit/ddt.asm"
;%include "boot_sector/32bit/print.asm"
%include "boot_sector/32bit/switch.asm"

[bits 16]
load_kernel:
    mov bx, MSG_LOAD_KERNEL
    call print
    call print_ln

    mov bx, KERNEL_OFFSET ; Read from disk and store in 0x1000
    mov dh, 54
    mov dl, [BOOT_DRIVE]
    call read_from_disk
    ret

[bits 32]
BEGIN_PM:
    mov ebx, MSG_PROT_MODE
    ;call print_string_pm
    call KERNEL_OFFSET ; Give control to the kernel
    jmp $ ; Stay here when the kernel returns control to us (if ever)


BOOT_DRIVE db 0 ; It is a good idea to store it in memory because 'dl' may get overwritten
MSG_REAL_MODE db "Started in 16-bit Real Mode", 0
MSG_PROT_MODE db "Landed in 32-bit Protected Mode", 0
MSG_LOAD_KERNEL db "Loading kernel into memory", 0

; padding
times (0x1b4 - ($-$$)) db 0    ; Pad For MBR Partition Table

UID times 10 db 0             ; Unique Disk ID
PT1:
    db 0x80 ; bootable

    db 0xff ; disk is bigger then 8gb
    db 0xff
    db 0xff

    db 0xeb

    db 0xff  ; disk is bigger then 8gb
    db 0xff
    db 0xff

    db 0
    db 0
    db 0
    db 0

    db 0xff
    db 0
    db 0
    db 0
PT2:
    db 0x0

    db 0xff  ; disk is bigger then 8gb
    db 0xff
    db 0xff

    db 0x40

    db 0xff  ; disk is bigger then 8gb
    db 0xff
    db 0xff

    db 0xff
    db 0
    db 0
    db 0

    db 0xff ; ~0.5 gb
    db 0xff
    db 0xff
    db 0x00

PT3 times 16 db 0             ; Third Partition Entry
PT4 times 16 db 0

dw 0xaa55
