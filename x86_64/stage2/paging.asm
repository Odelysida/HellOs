BITS 16

; --- Define -------------------------------------------------------------------
%define PAGE_PRESENT (1 << 0)
%define PAGE_WRITE   (1 << 1)
%define CODE_SEG     0x0008
%define PAGING_DATA  0x9000

; --- Initialized Data ---------------------------------------------------------

;****************************************************************************************;
; Global Descriptor Table (GDT)                                                          ;
;****************************************************************************************;
; The Global Descriptor Table (GDT) is a data structure used by x86-family processors    ;
; (starting with the 80286) in order to define the characteristics of the various memory ;
; areas (segments) used during program execution, including the base address, the size,  ;
; and access privileges like executability and writability.                              ;
;****************************************************************************************;
GDT:
    .Null:
        dq 0x0000000000000000        ; Null Descriptor
    .Code:
        dq 0x00209A0000000000        ; 64-bit code descriptor (exec/read)
        dq 0x0000920000000000        ; 64-bit data descriptor (read/write)
     ALIGN 4
        dw 0                         ; padding (to make the AoGDT field aligned a 4byte boundary)

    .Pointer:
        dw $ - GDT - 1               ; 16-bit size (limit) of GDT.
        dd GDT                       ; 32-bit Base Address of GDT. (CPU will zero extend to 64-bit)


;---Code------------------------------------------------------------------------
Prepare_paging:
;*******************************************************************************************;
; Prepare paging                                                                            ;
;-------------------------------------------------------------------------------------------;
; ES:EDI Should point to a valid page-aligned 16KiB buffer, for the PML4, PDPT, PD and a PT.;
; SS:ESP Should point to memory that can be used as a small (1 uint32_t) stack.             ;
;*******************************************************************************************;
    mov edi, PAGING_DATA;               ; Point edi to a free space to create the paging structures

    ; Zero out the 16KiB buffer. since we are doing a rep stosd, count should be bytes/4
    push di                             ; REP STOSD alters DI
    mov ecx, 0x1000
    xor eax, eax
    cld
    rep stosd
    pop di                              ; Get DI back

    ; Build the Page Map Level 4. ES:DI Points to the Page Map Lebel 4 table
    lea eax, [es:di + 0x1000]           ; EAX = Address of the Page Directory Pointer Table
    or eax, PAGE_PRESENT | PAGE_WRITE   ; OR EAX with the flags (present flag, writeable flag).
    mov [es:di], eax                    ; store the value of EAX as the first PML4E

    ; Build the Page Directory Pointer Table
    lea eax, [es:di + 0x2000]           ; Put the address of the Page Directory in to EAX
    or eax, PAGE_PRESET | PAGE_WRITE    ; OR EAX with the flags (present flaf, writeable flag).
    mov [es:di + 0x1000], eax           ; Store the value of EAX as the first PDPTE

    ; Build the Page Directory
    lea eax, [es:di + 0x3000]           ; put the address of the page table in to EAX
    or eax, PAGE_PRESENT | PAGE_WRITE   ; OR EAX with the flags (present flag, writable flag).
    mov [es:di + 0x2000], eax           ; store to value of EAX as the first PDE

    push di                             ; save DI for the time being
    lea di, [di + 0x3000]               ; point DI to the page table.
    mov eax, PAGE_PRESENT | PAGE_WRITE  ; Move the flags into EAX - and point it to 0x0000

    ; Build the Page Table.
    .LoopPageTable:
        mov [es:di], eax
        add eax, 0x1000
        add di, 8
        cmp eax, 0x200000               ; If we did all 2MiB, end.
        jb .LoopPageTable

    pop di                              ; restore DI
    ret

