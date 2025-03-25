BITS 16

; --- Initialized data -----------------------------------------------------
stage2_message dw 19
db 'Entering Stage 2...'
long_mode_supported_message dw 23
db 'Long mode is supported.'
long_mode_not_supported_message dw 27
db 'Long mode is not supported.'
prepare_paging_message dw 17
db 'Preparing Paging.'
remapping_pic_message dw 14
db 'Remapping PIC.'
executing_kernel_message dw 18
db 'Executing Kernel.'


; --- Code -----------------------------------------------------------------
setup_segments2:
    ; Print "entering Stage2..." Message
    mov si, stage2_message
    call Real_mode_println

    ; Check if long mode is supported
    call Is_longmode_supported
    test eax, eax
    jz .long_mode_not_supported
    mov si, long_mode_supported_message
    call Real_mode_println

    ; Enable Gate A20
    call Enable_A20
    ; Prepare paging
    mov si, prepare_paging_message
    call Real_mode_println

    call Prepare_paging


    ; Remap PIC
    mov si, remapping_pic_message
    call Real_mode_println
    call Remap_PIC
    ; Enter long mode
    call Enter_long_mode

    .long_mode_not_supported:
        mov si, long_mode_not_supported_message
        call Real_mode_println
       .halt: hlt ; Infinite loop. 
        jmp .halt ; (It prevents us from going off in memory and executing junk).


; Includes 

%include "stage2/a20.asm"
%include "stage2/paging.asm"
%include "stage2/pic.asm"
%include "stage2/longmode.asm"
