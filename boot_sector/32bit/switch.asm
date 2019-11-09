[bits 16]
switch_to_pm:
    cli ; clear interrupt enable bit
    lgdt [gdt_descriptor] ; load gdt

    mov eax, cr0
    or eax, 0x1
    mov cr0, eax
    ; set protected mode flag
    jmp CODE_SEG:init_pm

[bits 32]
init_pm:  ; we are now using 32-bit instructions
    mov ax, DATA_SEG ; 5. update the segment registers
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov ebp, 0x90000 ; 6. update the stack right at the top of the free space
    mov esp, ebp

    call BEGIN_PM ; 7. Call a well-known label with useful code

