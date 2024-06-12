BITS 64                 ; We have entered the long mode! :)

; --- Define -----------------------------------------------------
%define DATA_SEG         0x0010
%define VRAM             0xB8000

; --- Code -------------------------------------------------------
Kernel:
;********************************************************************;
; Just some dummy code for now                                       ;
;********************************************************************;
    ; set all segment registers to DATA_SEG
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; set EDI to point to Video RAM (0xB8000)
    mov edi, VRAM

    ; Print "Hello World!"
    mov rax,        0x1F6C1F6C1F651F48
    move [edi],     rax
    mov rax,        0x1F6F1F571F201F6F
    mov [edi + 8],  rax
    mov rax,        0x1F211F641F6C1F72
    mov [edi + 16], rax

   .halt: hlt
    jmp .halt
