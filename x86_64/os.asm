stage1_start:           ; Define start label for stage1
stage1_entrypoint:
    jmp 0x0000:setup_segments
    times 90 db 0           ; BPB (Bios Parameter Block)
    %include "stage1/bootstage1.asm"
stage1_end:

stage2_start:
stage2_entrypoint:
    jmp 0x0000:setup_segments2
    %include "stage2/bootstage2.asm"
    align 512, db 0    
stage2_end:

kernel_start:
kernel_entrypoint:
    jmp 0x0900:kernel_entry
    %include "kernel/kernel.asm"
    align 512, db 0
kernel_end:
