; HellOS Kernel Entry Point
; This file bridges the bootloader and the C kernel

[bits 32]

section .text
global _start                   ; Entry point
extern kernel_main              ; C kernel main function

_start:
    ; We're now in 32-bit protected mode
    ; Set up proper segment registers (consistent with unified bootloader)
    mov ax, 0x10               ; Data segment selector (matches bootloader GDT)
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    
    ; Set up stack at a safe location (consistent with bootloader)
    mov esp, 0x90000           ; Set stack pointer
    mov ebp, esp
    
    ; Clear screen with hellish message
    call clear_screen_hell
    
    ; Debug: Show that we're in kernel entry
    call show_kernel_entry_debug
    
    ; Call C kernel main function
    call kernel_main
    
    ; If kernel returns, halt the system
    cli
    hlt
    jmp $

; Clear screen and show hellish boot message
clear_screen_hell:
    ; Clear VGA text buffer with black background, red text
    mov edi, 0xB8000           ; VGA text buffer
    mov ecx, 80 * 25           ; Screen size (80x25 characters)
    mov ax, 0x0C20             ; Black background (0x0), red text (0xC), space (0x20)
    rep stosw                  ; Fill screen
    ret

; Show kernel entry debug message
show_kernel_entry_debug:
    mov edi, 0xB8000           ; VGA text buffer
    mov esi, kernel_entry_msg
    mov ah, 0x0C               ; Black background, red text
    
kernel_entry_print_loop:
    lodsb                      ; Load character from string
    test al, al                ; Check for null terminator
    jz kernel_entry_print_done
    stosw                      ; Store character and attribute
    jmp kernel_entry_print_loop
    
kernel_entry_print_done:
    ret

section .data
kernel_entry_msg: db 'Kernel Entry Point Reached! Calling kernel_main...', 0 