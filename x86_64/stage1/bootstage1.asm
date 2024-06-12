BITS 16         ; ON x86, BIOS runs in 16 BIT Real Mode.
ORG 0x7C00      ; We are loaded/booted by BIOS into this Memory Address

Stage1_entrypoint:              ; Main Entrypoint where BIOS leaves. Depending on Bios JMPs to 0x0000:0x7C00 while others on 0x07C0:0x0000. Far Jmp to accomodate 
    jmp 0x0000:.setup_segments
    .setup_segments:
        ; Set all segment registers to zero
        xor ax, ax
        mov ss, ax
        mov ds, ax
        mov es, ax
        mov fs, ax
        mov gs, ax

        ; Set up TemporayStack so growing start below stage1_entrypoint (i.e the stack base location at 0:0x7C00)

        mov sp, Stage1_entrypoint
        ; clear direction flag
        cld

    ; load stage2 from disk into RAM
    mov [disk], dl                                      ; storing disk number of the booted device
    mov ax, (stage2_start - stage1_start)/512           ; ax: start sector
    mov cx, (kernel_end-stage2_start)/512               ; cx: number of sectors (512 Bytes) to read
    mov bx, stage2_start                                ; bx: offset of buffer
    xor dx, dx                                          ; dx: segment of buffer
    call Real_mode_read_disk

    ; Print first stage finished
    mov si, stage1_message
    call Real_mode_println

    ; Jump to the entry Point of stage 2 (todo)
    ;;; jmp Stage2_entrypoint

    .halt: hlt          ; Infinite Loop
    jmp .halt           ; (Prevents from going off in Memory!)


; Include
%include "stage1/disk.asm"
%include "stage1/print.asm"



times 510-($-$$) db 0 ; Padding as always
dw 0xAA55             ; Last two Bytes of Boot Sector should have the 0xAA55 Signature.

; Otherwise Bios will throw a error Message that it didn`t find a bootable Disk.

