BITS 16

; --- Initialized data ------------------------------------------------------

newline dw 2
db 13, 10 ; \r\n

stage1_message dw 17
db 'Stage 1 finished.'


; --- Code ------------------------------------------------------------------

Real_mode_print:
;*********************************************************************************;
; Prints a string (in real mode)                                                  ;
;---------------------------------------------------------------------------------;
; si: pointer to string (first 16 bits = the number of characters in the string.) ;  
;*********************************************************************************;

    push ax
    push cx
    push si
    mov  cx, word [si]      ; first 16 bits = number of chatacters in string
    add si, 2
    .string_loop:
        lodsb 
        mov ah, 0eh
        int 10h
    loop .string_loop, cx
    pop si
    pop cx
    pop ax
    ret


Real_mode_println:
;***********************************************************;
; Prints a string (in real mode) and a newline (\r\n)       ;
;-----------------------------------------------------------;
; si: pointer to string                                     ;
; (first 16 bits = the number of characters in the string.) ;  
;***********************************************************;
    push si
    call Real_mode_print
    mov si, newline
    call Real_mode_print
    pop si
    ret

BITS 64

; --- Initialized data for Long Mode ----------------------------------------

; Video memory location
%define VRAM 0xB8000

; --- Long mode functions ---------------------------------------------------

Long_mode_print:
;*********************************************************************************;
; Prints a string in long mode by writing directly to video memory                ;
;---------------------------------------------------------------------------------;
; rsi: pointer to string (first 16 bits = the number of characters in the string) ;
;*********************************************************************************;
    push rax
    push rcx
    push rdi
    mov  rcx, word [rsi]    ; first 16 bits = number of characters in string
    add  rsi, 2             ; Move past the length bytes
    mov  rdi, VRAM          ; Point to video memory

    .string_loop:
        lodsb               ; Load byte from [rsi] into al, increment rsi
        stosb               ; Store al into [rdi], increment rdi
        mov byte [rdi], 0x07 ; Set attribute (light gray on black)
        inc rdi
    loop .string_loop

    pop rdi
    pop rcx
    pop rax
    ret

Long_mode_println:
;***********************************************************;
; Prints a string (in long mode) and a newline (\r\n)       ;
;-----------------------------------------------------------;
; rsi: pointer to string                                    ;
; (first 16 bits = the number of characters in the string.) ;
;***********************************************************;
    push rsi
    call Long_mode_print
    mov rsi, newline         ; newline is still defined with 16-bit length
    call Long_mode_print
    pop rsi
    ret

; Switch back to 16-bit mode if needed
BITS 16