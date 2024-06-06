;   Kernel main assembly - OS Pipeline
;   Basic implementation puts string to SI register and loops printing of it
;   just testing nasm with i386 target

    BITS 16

start: 
    mov ax, 07C0h                                   ; Set up 4k stack space after bootloader
    add ax, 288                                     ; (4096 + 512) / 16 bytes per paragraph
    mov ss, ax
    mov sp, 4096
    
    mov ax, 07c0h                                   ; set data segment to where we're loaded
    mov ds, ax


    mov si, text_string                             ; put string position into SI
    call print_string                               ; Call our string-printig routine

    jmp $                                           ; jmp here -> infinite loop 

    text_string db 'still alive, fck that hi-level stuff!', 0

print_string:                                       ; routine_ output string in SI to screen
    mov ah, 0Eh                                     ; int 10h 'print char' function 

.repeat:
    lodsb                                           ; get character from string
    cmp al, 0                       
    je .done                                        ; If char is zero, end of string
    int 10h
    jmp .repeat

.done:
    ret


    times 510-($-$$) db 0                           ; pad remainder of sector with 0s
    dw 0xAA55                                       ; The standard PC boot signature

