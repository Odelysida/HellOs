BITS 16

;---Code-----------------------------------------------------------------------
Is_longmode_supported:
;********************************************************************;
; Check if Long mode is supported                                    ;
;--------------------------------------------------------------------;
; Returns: eax = 0 if Long mode is NOT supported, else non-zero.     ;
;********************************************************************;
    mov eax, 0x80000000             ; Test if extended processor info in available
    cpuid
    cmp eax, 0x80000001             
    jb .not_supported

    mov eax, 0x80000001             ; After calling CPUID with EAX = 0x80000001
    cpuid                           ; all AMD64 compliant processors have the longmode-capable-bit
    test edx, (1 << 29)             ; (bit 29) turned on in the EDX (Extended Feature Flags)

    jz .not_supported
    ret


   .not_supported:
        xor eax, eax
        ret


Enter_long_mode:
;********************************************************************;
; Enter long mode                                                    ;
;********************************************************************;
; Set up PAE and PGE
    mov eax, 0xA0         ; PAE (bit 5) + PGE (bit 7)
    mov cr4, eax

    ; Point CR3 to PML4
    mov eax, PAGING_DATA
    mov cr3, eax

    ; Enable LME in EFER
    mov ecx, 0xC0000080
    rdmsr
    or eax, 0x100         ; LME (bit 8)
    wrmsr

    ; Load GDT *before* enabling paging
    lgdt [GDT.Pointer]

    ; Enable paging and protected mode
    mov eax, cr0
    or eax, 0x80000001    ; Paging + Protected Mode
    mov cr0, eax

    ; Far jump to 64-bit mode
    jmp CODE_SEG:kernel_entry

