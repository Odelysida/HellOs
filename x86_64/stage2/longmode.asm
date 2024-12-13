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
    mov edi, PAGING_DATA            ; point edi at the PAGING_DATA
    mov eax, 10100000b              ; set the PAE and PGE bit
    mov cr4, eax
    mov edx, edi                    ; Point CR3 at the PML4
    mov cr3, edx                    
    mov ecx, 0xC0000080             ; Read from the EFER MSR
    rdmsr                   
    or eax, 0x00000100              ; Set the LME bit
    wrmsr
    mov ebx, cr0                    ; activate long mode
    or ebx, 0x80000001              ; by enabling paging and protection simultaneously
    mov cr0, ebx
    lgdt [GDT.Pointer]              ; load GDT.Pointer
    jmp CODE_SEG:Kernel             ; Load cs with 64 bit segment and flush the instruction cache.

