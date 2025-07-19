BITS 16         ; ON x86, BIOS runs in 16 BIT Real Mode.
ORG 0x7C00      ; We are loaded/booted by BIOS into this Memory Address

           
setup_segments:
    xor ax, ax          ; Clear AX
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov sp, 0x7C00      ; Set up stack pointer
    cld                 ; Clear direction flag

    ; Load stage2 from disk into RAM
    mov [disk], dl
    mov ax, 1           ; Starting sector
    mov cx, 2           ; Number of sectors to read (pre-calculated)
    mov bx, stage2_start
    xor dx, dx
    call Real_mode_read_disk

    ; Print first stage finished
    mov si, stage1_message
    call Real_mode_println

    jmp stage2_entrypoint
    ; Infinite loop
halt:
    hlt
    jmp halt

    ; Set up TemporayStack so growi
; Include
%include "stage1/disk.asm"
%include "stage1/print.asm"



times 510-($-$$) db 0 ; Padding as always
db 0x55                 ; Boot signature byte 1
db 0xAA                 ; Boot signature byte 2
; Last two Bytes
; Otherwise Bios will throw a error Message that it didn`t find a bootable Disk.

