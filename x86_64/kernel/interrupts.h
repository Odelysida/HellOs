/*
 * HellOS Interrupt System Header
 * Interrupt handling definitions and structures
 */

#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include <stdint.h>
#include <stdbool.h>

// Forward declarations
typedef struct idt_entry_s idt_entry_t;
typedef struct idt_ptr_s idt_ptr_t;
typedef struct interrupt_stats_s interrupt_stats_t;

// Interrupt system functions
void init_interrupt_system(void);
void process_interrupts(void);
void set_idt_entry(int num, uint32_t handler, uint16_t selector, uint8_t flags);
void init_pic(void);
void load_idt(idt_ptr_t* idt_ptr);

// Interrupt control functions
void enable_interrupts(void);
void disable_interrupts(void);
bool interrupts_enabled(void);

// Interrupt handlers
void default_interrupt_handler(void);
void exception_handler(uint32_t exception_num, uint32_t error_code);
void hardware_interrupt_handler(uint32_t irq_num);
void process_keyboard_input(uint8_t scancode);

// Statistics
interrupt_stats_t* get_interrupt_stats(void);

#endif // INTERRUPTS_H 