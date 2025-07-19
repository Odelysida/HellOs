/*
 * HellOS Interrupt System
 * The infernal interrupt and exception handling
 */

#include "kernel.h"
#include "interrupts.h"
#include <stdint.h>

// IDT constants
#define IDT_SIZE 256
#define INTERRUPT_GATE 0x8E
#define TRAP_GATE 0x8F

// Exception constants
#define EXCEPTION_DIVIDE_BY_ZERO    0
#define EXCEPTION_DEBUG             1
#define EXCEPTION_NMI               2
#define EXCEPTION_BREAKPOINT        3
#define EXCEPTION_OVERFLOW          4
#define EXCEPTION_BOUND_RANGE       5
#define EXCEPTION_INVALID_OPCODE    6
#define EXCEPTION_DEVICE_NOT_AVAIL  7
#define EXCEPTION_DOUBLE_FAULT      8
#define EXCEPTION_INVALID_TSS       10
#define EXCEPTION_SEGMENT_NOT_PRES  11
#define EXCEPTION_STACK_FAULT       12
#define EXCEPTION_GENERAL_PROT      13
#define EXCEPTION_PAGE_FAULT        14
#define EXCEPTION_FPU_ERROR         16
#define EXCEPTION_ALIGNMENT_CHECK   17
#define EXCEPTION_MACHINE_CHECK     18
#define EXCEPTION_SIMD_FP_ERROR     19

// Hardware interrupt constants
#define IRQ_TIMER           32
#define IRQ_KEYBOARD        33
#define IRQ_CASCADE         34
#define IRQ_COM2            35
#define IRQ_COM1            36
#define IRQ_LPT2            37
#define IRQ_FLOPPY          38
#define IRQ_LPT1            39
#define IRQ_RTC             40
#define IRQ_FREE1           41
#define IRQ_FREE2           42
#define IRQ_FREE3           43
#define IRQ_MOUSE           44
#define IRQ_FPU             45
#define IRQ_ATA_PRIMARY     46
#define IRQ_ATA_SECONDARY   47

// IDT entry structure (32-bit)
struct idt_entry_s {
    uint16_t offset_low;    // Offset bits 0-15
    uint16_t selector;      // Code segment selector
    uint8_t zero;           // Must be zero
    uint8_t type_attr;      // Type and attributes
    uint16_t offset_high;   // Offset bits 16-31
} __attribute__((packed));

// IDT pointer structure (32-bit)
struct idt_ptr_s {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

// Interrupt statistics (32-bit)
struct interrupt_stats_s {
    uint32_t total_interrupts;
    uint32_t exceptions;
    uint32_t hardware_interrupts;
    uint32_t software_interrupts;
    uint32_t spurious_interrupts;
    uint32_t timer_ticks;
    uint32_t keyboard_interrupts;
};

// Global interrupt state
static idt_entry_t idt[IDT_SIZE];
static idt_ptr_t idt_ptr;
static interrupt_stats_t interrupt_stats = {0};
static bool interrupts_initialized = false;

// Interrupt handler function pointer type
typedef void (*interrupt_handler_t)(void);

// Forward declarations
void default_interrupt_handler(void);
void exception_handler(uint32_t exception_num, uint32_t error_code);
void hardware_interrupt_handler(uint32_t irq_num);

// Exception handler functions
void divide_by_zero_handler(void);
void debug_handler(void);
void nmi_handler(void);
void breakpoint_handler(void);
void overflow_handler(void);
void bound_range_handler(void);
void invalid_opcode_handler(void);
void device_not_available_handler(void);
void double_fault_handler(void);
void invalid_tss_handler(void);
void segment_not_present_handler(void);
void stack_fault_handler(void);
void general_protection_handler(void);
void page_fault_handler(void);
void fpu_error_handler(void);
void alignment_check_handler(void);
void machine_check_handler(void);
void simd_fp_error_handler(void);

// Hardware interrupt handlers
void timer_interrupt_handler(void);
void keyboard_interrupt_handler(void);

/*
 * Initialize the interrupt system
 */
void init_interrupt_system(void) {
    // Initialize IDT
    for (int i = 0; i < IDT_SIZE; i++) {
        set_idt_entry(i, (uint32_t)default_interrupt_handler, 0x08, INTERRUPT_GATE);
    }
    
    // Set up exception handlers
    set_idt_entry(EXCEPTION_DIVIDE_BY_ZERO, (uint32_t)divide_by_zero_handler, 0x08, TRAP_GATE);
    set_idt_entry(EXCEPTION_DEBUG, (uint32_t)debug_handler, 0x08, TRAP_GATE);
    set_idt_entry(EXCEPTION_NMI, (uint32_t)nmi_handler, 0x08, INTERRUPT_GATE);
    set_idt_entry(EXCEPTION_BREAKPOINT, (uint32_t)breakpoint_handler, 0x08, TRAP_GATE);
    set_idt_entry(EXCEPTION_OVERFLOW, (uint32_t)overflow_handler, 0x08, TRAP_GATE);
    set_idt_entry(EXCEPTION_BOUND_RANGE, (uint32_t)bound_range_handler, 0x08, TRAP_GATE);
    set_idt_entry(EXCEPTION_INVALID_OPCODE, (uint32_t)invalid_opcode_handler, 0x08, TRAP_GATE);
    set_idt_entry(EXCEPTION_DEVICE_NOT_AVAIL, (uint32_t)device_not_available_handler, 0x08, TRAP_GATE);
    set_idt_entry(EXCEPTION_DOUBLE_FAULT, (uint32_t)double_fault_handler, 0x08, TRAP_GATE);
    set_idt_entry(EXCEPTION_INVALID_TSS, (uint32_t)invalid_tss_handler, 0x08, TRAP_GATE);
    set_idt_entry(EXCEPTION_SEGMENT_NOT_PRES, (uint32_t)segment_not_present_handler, 0x08, TRAP_GATE);
    set_idt_entry(EXCEPTION_STACK_FAULT, (uint32_t)stack_fault_handler, 0x08, TRAP_GATE);
    set_idt_entry(EXCEPTION_GENERAL_PROT, (uint32_t)general_protection_handler, 0x08, TRAP_GATE);
    set_idt_entry(EXCEPTION_PAGE_FAULT, (uint32_t)page_fault_handler, 0x08, TRAP_GATE);
    set_idt_entry(EXCEPTION_FPU_ERROR, (uint32_t)fpu_error_handler, 0x08, TRAP_GATE);
    set_idt_entry(EXCEPTION_ALIGNMENT_CHECK, (uint32_t)alignment_check_handler, 0x08, TRAP_GATE);
    set_idt_entry(EXCEPTION_MACHINE_CHECK, (uint32_t)machine_check_handler, 0x08, TRAP_GATE);
    set_idt_entry(EXCEPTION_SIMD_FP_ERROR, (uint32_t)simd_fp_error_handler, 0x08, TRAP_GATE);
    
    // Set up hardware interrupt handlers
    set_idt_entry(IRQ_TIMER, (uint32_t)timer_interrupt_handler, 0x08, INTERRUPT_GATE);
    set_idt_entry(IRQ_KEYBOARD, (uint32_t)keyboard_interrupt_handler, 0x08, INTERRUPT_GATE);
    
    // Initialize PIC (Programmable Interrupt Controller)
    init_pic();
    
    // Load IDT
    idt_ptr.limit = sizeof(idt) - 1;
    idt_ptr.base = (uint32_t)&idt;
    load_idt(&idt_ptr);
    
    interrupts_initialized = true;
}

/*
 * Set an IDT entry (32-bit)
 */
void set_idt_entry(int num, uint32_t handler, uint16_t selector, uint8_t flags) {
    idt[num].offset_low = handler & 0xFFFF;
    idt[num].selector = selector;
    idt[num].zero = 0;
    idt[num].type_attr = flags;
    idt[num].offset_high = (handler >> 16) & 0xFFFF;
}

/*
 * Initialize the PIC
 */
void init_pic(void) {
    // Initialize master PIC
    outb(0x20, 0x11); // ICW1: Initialize
    outb(0x21, 0x20); // ICW2: Master PIC vector offset (32)
    outb(0x21, 0x04); // ICW3: Tell master PIC that there is a slave PIC at IRQ2
    outb(0x21, 0x01); // ICW4: 8086 mode
    
    // Initialize slave PIC
    outb(0xA0, 0x11); // ICW1: Initialize
    outb(0xA1, 0x28); // ICW2: Slave PIC vector offset (40)
    outb(0xA1, 0x02); // ICW3: Tell slave PIC its cascade identity
    outb(0xA1, 0x01); // ICW4: 8086 mode
    
    // Enable all interrupts except cascade
    outb(0x21, 0xFB); // Master PIC mask (enable cascade)
    outb(0xA1, 0xFF); // Slave PIC mask (disable all)
}

/*
 * Load IDT
 */
void load_idt(idt_ptr_t* idt_ptr) {
    __asm__ volatile ("lidt %0" : : "m" (*idt_ptr));
}

/*
 * Process interrupts
 */
void process_interrupts(void) {
    if (!interrupts_initialized) {
        return;
    }
    
    // This function is called from the main kernel loop
    // Hardware interrupts are handled automatically by the IDT
    // This is where we can do any periodic interrupt processing
    
    // Update interrupt statistics
    interrupt_stats.total_interrupts++;
}

/*
 * Default interrupt handler
 */
void default_interrupt_handler(void) {
    interrupt_stats.spurious_interrupts++;
    
    // Send EOI to PIC
    outb(0x20, 0x20);
    outb(0xA0, 0x20);
}

/*
 * Exception handler
 */
void exception_handler(uint32_t exception_num, uint32_t error_code) {
    (void)error_code; // Suppress unused parameter warning
    interrupt_stats.exceptions++;
    
    // Display hellish exception message
    const char* exception_names[] = {
        "Division by Zero - The void consumes all",
        "Debug - The demons are watching",
        "Non-Maskable Interrupt - The abyss calls",
        "Breakpoint - Pause in the infernal realm",
        "Overflow - The gates of hell overflow",
        "Bound Range Exceeded - Beyond the infernal bounds",
        "Invalid Opcode - Unknown incantation",
        "Device Not Available - The demon sleeps",
        "Double Fault - The realm collapses",
        "Coprocessor Segment Overrun - Ancient evil",
        "Invalid TSS - Corrupted soul",
        "Segment Not Present - Lost in the void",
        "Stack Fault - The stack burns",
        "General Protection Fault - The guardian blocks",
        "Page Fault - Memory banished to purgatory",
        "Reserved - The unknown terror",
        "FPU Error - Numbers from hell",
        "Alignment Check - Misaligned with evil",
        "Machine Check - Hardware possessed",
        "SIMD FP Error - Floating point damnation"
    };
    
    if (exception_num < 20) {
        kernel_panic(exception_names[exception_num]);
    } else {
        kernel_panic("Unknown exception from the depths of hell");
    }
}

/*
 * Hardware interrupt handler
 */
void hardware_interrupt_handler(uint32_t irq_num) {
    interrupt_stats.hardware_interrupts++;
    
    // Handle specific IRQs
    switch (irq_num) {
        case IRQ_TIMER:
            timer_interrupt_handler();
            break;
        case IRQ_KEYBOARD:
            keyboard_interrupt_handler();
            break;
        default:
            // Unknown hardware interrupt
            break;
    }
    
    // Send EOI to PIC
    if (irq_num >= 40) {
        outb(0xA0, 0x20); // Send EOI to slave PIC
    }
    outb(0x20, 0x20); // Send EOI to master PIC
}

// Exception handlers
void divide_by_zero_handler(void) {
    exception_handler(EXCEPTION_DIVIDE_BY_ZERO, 0);
}

void debug_handler(void) {
    exception_handler(EXCEPTION_DEBUG, 0);
}

void nmi_handler(void) {
    exception_handler(EXCEPTION_NMI, 0);
}

void breakpoint_handler(void) {
    exception_handler(EXCEPTION_BREAKPOINT, 0);
}

void overflow_handler(void) {
    exception_handler(EXCEPTION_OVERFLOW, 0);
}

void bound_range_handler(void) {
    exception_handler(EXCEPTION_BOUND_RANGE, 0);
}

void invalid_opcode_handler(void) {
    exception_handler(EXCEPTION_INVALID_OPCODE, 0);
}

void device_not_available_handler(void) {
    exception_handler(EXCEPTION_DEVICE_NOT_AVAIL, 0);
}

void double_fault_handler(void) {
    exception_handler(EXCEPTION_DOUBLE_FAULT, 0);
}

void invalid_tss_handler(void) {
    exception_handler(EXCEPTION_INVALID_TSS, 0);
}

void segment_not_present_handler(void) {
    exception_handler(EXCEPTION_SEGMENT_NOT_PRES, 0);
}

void stack_fault_handler(void) {
    exception_handler(EXCEPTION_STACK_FAULT, 0);
}

void general_protection_handler(void) {
    exception_handler(EXCEPTION_GENERAL_PROT, 0);
}

void page_fault_handler(void) {
    exception_handler(EXCEPTION_PAGE_FAULT, 0);
}

void fpu_error_handler(void) {
    exception_handler(EXCEPTION_FPU_ERROR, 0);
}

void alignment_check_handler(void) {
    exception_handler(EXCEPTION_ALIGNMENT_CHECK, 0);
}

void machine_check_handler(void) {
    exception_handler(EXCEPTION_MACHINE_CHECK, 0);
}

void simd_fp_error_handler(void) {
    exception_handler(EXCEPTION_SIMD_FP_ERROR, 0);
}

// Hardware interrupt handlers
void timer_interrupt_handler(void) {
    interrupt_stats.timer_ticks++;
    
    // Timer interrupt - used for scheduling
    // This is where we would do process switching
    
    // Play periodic hellish sounds
    if (interrupt_stats.timer_ticks % 1000 == 0) {
        // Every 1000 ticks, make a subtle demonic sound
        play_note(2, NOTE_C1, WAVE_SAW, 50);
    }
}

void keyboard_interrupt_handler(void) {
    interrupt_stats.keyboard_interrupts++;
    
    // Read scancode from keyboard
    uint8_t scancode = inb(0x60);
    
    // Process keyboard input
    process_keyboard_input(scancode);
}

/*
 * Process keyboard input
 */
void process_keyboard_input(uint8_t scancode) {
    // Simple scancode to ASCII conversion
    static const char scancode_to_ascii[] = {
        0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
        '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
        0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
        0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
        '*', 0, ' '
    };
    
    // Only process key press events (not releases)
    if (!(scancode & 0x80) && scancode < sizeof(scancode_to_ascii)) {
        char ascii = scancode_to_ascii[scancode];
        if (ascii) {
            // Send to shell
            process_shell_input(ascii);
        }
    }
}

/*
 * Get interrupt statistics
 */
interrupt_stats_t* get_interrupt_stats(void) {
    return &interrupt_stats;
}

/*
 * Enable interrupts
 */
void enable_interrupts(void) {
    __asm__ volatile ("sti");
}

/*
 * Disable interrupts
 */
void disable_interrupts(void) {
    __asm__ volatile ("cli");
}

/*
 * Check if interrupts are enabled
 */
bool interrupts_enabled(void) {
    uint32_t flags;
    __asm__ volatile ("pushfl; popl %0" : "=r" (flags));
    return (flags & 0x200) != 0;
} 