/*
 * HellOS Memory Layout Definitions
 * Unified memory layout constants for bootloader and kernel
 */

#ifndef MEMORY_LAYOUT_H
#define MEMORY_LAYOUT_H

// Memory Layout Constants
#define BOOTLOADER_LOAD_ADDR    0x7C00      // Where BIOS loads bootloader
#define BOOTLOADER_STACK_ADDR   0x7C00      // Stack grows down from bootloader
#define KERNEL_LOAD_ADDR        0x8000      // Where kernel is loaded (32KB)
#define KERNEL_STACK_ADDR       0x90000     // Kernel stack (576KB)
#define KERNEL_MAX_SIZE         0x10000     // Maximum kernel size (64KB)

// Memory Regions
#define REAL_MODE_IVT_START     0x0000      // Real mode interrupt vector table
#define REAL_MODE_IVT_END       0x03FF      // End of IVT
#define BIOS_DATA_AREA_START    0x0400      // BIOS data area
#define BIOS_DATA_AREA_END      0x04FF      // End of BIOS data area
#define CONVENTIONAL_RAM_START  0x0500      // Start of conventional RAM
#define VIDEO_MEMORY_START      0xA0000     // Start of video memory
#define VIDEO_MEMORY_END        0xBFFFF     // End of video memory
#define BIOS_ROM_START          0xC0000     // Start of BIOS ROM area
#define EXTENDED_BIOS_START     0xE0000     // Extended BIOS area
#define SYSTEM_BIOS_START       0xF0000     // System BIOS

// VGA Text Mode
#define VGA_TEXT_BUFFER         0xB8000     // VGA text mode buffer
#define VGA_TEXT_WIDTH          80          // Characters per line
#define VGA_TEXT_HEIGHT         25          // Lines on screen

// Stack Configuration
#define STACK_SIZE              0x8000      // 32KB stack
#define STACK_GUARD_SIZE        0x1000      // 4KB guard page
#define KERNEL_STACK_BASE       (KERNEL_STACK_ADDR - STACK_SIZE)
#define KERNEL_STACK_TOP        KERNEL_STACK_ADDR

// Memory Safety Checks
#define MEMORY_SAFE_START       KERNEL_LOAD_ADDR
#define MEMORY_SAFE_END         (KERNEL_LOAD_ADDR + KERNEL_MAX_SIZE)
#define STACK_SAFE_START        KERNEL_STACK_BASE
#define STACK_SAFE_END          KERNEL_STACK_TOP

// Segment Selectors (consistent with bootloader GDT)
#define NULL_SEG                0x00        // Null segment
#define CODE_SEG                0x08        // Code segment selector
#define DATA_SEG                0x10        // Data segment selector

// Page Directory/Table Addresses (for future paging implementation)
#define PAGE_DIRECTORY_ADDR     0x100000    // 1MB mark
#define PAGE_TABLE_ADDR         0x101000    // After page directory

// Heap Management (for future implementation)
#define HEAP_START              0x200000    // 2MB mark
#define HEAP_INITIAL_SIZE       0x100000    // 1MB initial heap

// Memory Layout Validation Macros
#define IS_VALID_KERNEL_ADDR(addr) \
    ((addr) >= MEMORY_SAFE_START && (addr) < MEMORY_SAFE_END)

#define IS_VALID_STACK_ADDR(addr) \
    ((addr) >= STACK_SAFE_START && (addr) < STACK_SAFE_END)

#define IS_KERNEL_OVERLAP(addr, size) \
    (((addr) < MEMORY_SAFE_END) && ((addr) + (size) > MEMORY_SAFE_START))

#define IS_STACK_OVERLAP(addr, size) \
    (((addr) < STACK_SAFE_END) && ((addr) + (size) > STACK_SAFE_START))

// Architecture Constants
#define ARCH_32BIT              1
#define ARCH_64BIT              0
#define CURRENT_ARCH            ARCH_32BIT

// Boot Protocol Constants
#define BOOT_SIGNATURE          0xAA55      // Boot sector signature
#define BOOT_SECTOR_SIZE        512         // Boot sector size in bytes
#define SECTOR_SIZE             512         // Standard sector size

#endif /* MEMORY_LAYOUT_H */ 