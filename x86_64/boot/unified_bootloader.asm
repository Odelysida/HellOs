; HellOS Unified Bootloader - Compact Version
; Fixed architecture bootloader that fits in 512 bytes

[BITS 16]
[ORG 0x7C00]

; Boot sector constants
KERNEL_LOAD_ADDR    equ 0x8000      ; Kernel load address
STACK_ADDR          equ 0x7C00      ; Stack below bootloader
KERNEL_SECTORS      equ 8           ; Number of sectors to read (4KB max)

start:
    ; Set up segments and stack
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, STACK_ADDR
    sti
    
    ; Save boot drive
    mov [boot_drive], dl
    
    ; Initialize serial port for debug output
    call init_serial_debug
    
    ; Send debug message to serial
    mov si, debug_boot_start
    call print_string_serial
    
    ; Clear screen - black background with red text
    mov ax, 0x0600
    mov bh, 0x0C        ; Black background (0x0), red text (0xC)
    mov cx, 0x0000
    mov dx, 0x184F
    int 0x10
    
    ; Print banner
    mov si, banner
    call print_string
    
    ; Load kernel
    call load_kernel
    
    ; Switch to protected mode
    call init_protected_mode
    
    ; Hang if we get here
    jmp hang

; Load kernel from disk sector 2
load_kernel:
    ; Debug message
    mov si, debug_kernel_loading
    call print_string_serial
    
    ; Read kernel from disk
    mov ah, 0x02                ; BIOS read sectors
    mov al, KERNEL_SECTORS      ; Number of sectors to read
    mov ch, 0                   ; Cylinder 0
    mov cl, 3           ; Sector 3 (kernel is at LBA sector 2, which is CHS sector 3)
    mov dh, 0                   ; Head 0
    mov dl, [boot_drive]        ; Drive number
    
    ; Set up buffer
    mov bx, KERNEL_LOAD_ADDR    ; Buffer address
    
    ; Read sectors
    int 0x13
    jc disk_error               ; Jump if carry flag set (error)
    
    ; Debug message
    mov si, debug_kernel_loaded
    call print_string_serial
    
    ; Print success message
    mov si, kernel_loaded_msg
    call print_string
    ret

disk_error:
    mov si, disk_error_msg
    call print_string
    jmp hang

; Initialize protected mode
init_protected_mode:
    ; Debug message
    mov si, debug_protected_mode
    call print_string_serial
    
    cli
    lgdt [gdt_descriptor]
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax
    jmp CODE_SEG:protected_mode_entry

; Print string (compact version)
print_string:
    pusha
    mov ah, 0x0E
    mov bl, 0x0C        ; Black background, red text
.loop:
    lodsb
    test al, al
    jz .done
    int 0x10
    jmp .loop
.done:
    popa
    ret

; Initialize serial port for debug output
init_serial_debug:
    mov dx, 0x3F8 + 3    ; COM1 + 3
    mov al, 0x80
    out dx, al           ; Enable DLAB
    mov dx, 0x3F8        ; COM1
    mov al, 0x03
    out dx, al           ; Set divisor (38400 baud)
    mov dx, 0x3F8 + 3    ; COM1 + 3
    mov al, 0x03
    out dx, al           ; 8N1
    ret

; Print string to serial port
print_string_serial:
    pusha
.loop:
    lodsb
    test al, al
    jz .done
    call print_char_serial
    jmp .loop
.done:
    popa
    ret

; Print single character to serial port
print_char_serial:
    mov dx, 0x3F8 + 5    ; COM1 + 5 (line status)
.wait:
    in al, dx
    and al, 0x20         ; Check if transmit buffer empty
    jz .wait
    mov dx, 0x3F8        ; COM1
    mov al, [si-1]       ; Get character
    out dx, al           ; Send character
    ret

; Hang system
hang:
    cli
    hlt
    jmp hang

; 32-bit protected mode entry
[BITS 32]
protected_mode_entry:
    ; Set up segments
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    
    ; Set up stack
    mov esp, 0x90000
    mov ebp, esp
    
    ; Clear screen in protected mode - black background, red text
    mov edi, 0xB8000
    mov ecx, 80 * 25
    mov eax, 0x0C200C20     ; Black background (0x0), red text (0xC), space character (0x20)
    rep stosd
    
    ; Print debug message before jumping to kernel
    mov edi, 0xB8000
    mov esi, debug_msg
    mov ah, 0x0C               ; Black background, red text
    
debug_print_loop:
    lodsb                      ; Load character from string
    test al, al                ; Check for null terminator
    jz jump_to_kernel
    stosw                      ; Store character and attribute
    jmp debug_print_loop
    
jump_to_kernel:
    ; Jump to kernel
    jmp KERNEL_LOAD_ADDR

; Global Descriptor Table
[BITS 16]
gdt_start:
    ; Null descriptor
    dd 0x0, 0x0

gdt_code:
    ; Code segment - 4GB flat
    dw 0xFFFF, 0x0000
    db 0x00, 10011010b, 11001111b, 0x00

gdt_data:
    ; Data segment - 4GB flat
    dw 0xFFFF, 0x0000
    db 0x00, 10010010b, 11001111b, 0x00

gdt_end:

; GDT descriptor
gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

; Segment selectors
CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

; Data
boot_drive: db 0

banner:
    db 13, 10, 'HellOS v2.0 - Infernal Boot', 13, 10
    db 'Loading kernel...', 13, 10, 0

kernel_loaded_msg:
    db 'Kernel loaded from disk!', 13, 10, 0

disk_error_msg:
    db 'DISK ERROR: Cannot load kernel!', 13, 10, 0

debug_msg:
    db 'Jumping to kernel at 0x8000...', 0

debug_boot_start:
    db '[BOOT] Starting...', 13, 10, 0

debug_kernel_loading:
    db '[BOOT] Loading...', 13, 10, 0

debug_kernel_loaded:
    db '[BOOT] Loaded', 13, 10, 0

debug_protected_mode:
    db '[BOOT] PM', 13, 10, 0

; Pad to 512 bytes and add boot signature
times 510-($-$$) db 0
dw 0xAA55 