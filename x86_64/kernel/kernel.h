/*
 * HellOS Kernel Header
 * Core kernel definitions and structures
 */

#ifndef KERNEL_H
#define KERNEL_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "memory_layout.h"

// Screen dimensions
#define SCREEN_WIDTH 680
#define SCREEN_HEIGHT 480

// Color definitions (5-bit palette, 32 colors)
#define COLOR_VOID_BLACK     0x00
#define COLOR_HELL_RED       0x01
#define COLOR_FLAME_ORANGE   0x02
#define COLOR_DARK_RED       0x03
#define COLOR_DEEP_CRIMSON   0x04
#define COLOR_SULFUR_YELLOW  0x05
#define COLOR_EMBER_GLOW     0x06
#define COLOR_SHADOW_GRAY    0x07
#define COLOR_BONE_WHITE     0x08
#define COLOR_BLOOD_MAROON   0x09
#define COLOR_FIRE_BRIGHT    0x0A
#define COLOR_ASH_GRAY       0x0B
#define COLOR_DEMON_PURPLE   0x0C
#define COLOR_TOXIC_GREEN    0x0D
#define COLOR_RUST_BROWN     0x0E
#define COLOR_MAGMA_RED      0x0F
// Additional colors 0x10-0x1F for extended palette

// Kernel status codes
typedef enum {
    KERNEL_STATUS_INITIALIZING,
    KERNEL_STATUS_DRIVERS_LOADED,
    KERNEL_STATUS_SHELL_ACTIVE,
    KERNEL_STATUS_RUNNING,
    KERNEL_STATUS_SHUTTING_DOWN,
    KERNEL_STATUS_HALTED,
    KERNEL_STATUS_PANIC
} kernel_status_t;

// Kernel state structure
typedef struct {
    uint64_t boot_time;
    uint64_t memory_size;
    kernel_status_t status;
    uint32_t process_count;
    uint32_t demon_count;  // Active system processes
} kernel_state_t;

// Audio note structure
typedef struct {
    uint16_t frequency;
    uint8_t waveform;
    uint16_t duration_ms;
} audio_note_t;

// Audio waveform types
#define WAVE_SINE     0
#define WAVE_SQUARE   1
#define WAVE_SAW      2

// Musical notes (frequencies in Hz)
#define NOTE_C1   33
#define NOTE_DS1  39
#define NOTE_G1   49
#define NOTE_C2   65
#define NOTE_DS2  78
#define NOTE_G2   98
#define NOTE_C3   131
#define NOTE_DS3  156
#define NOTE_G3   196

// Window structure
typedef struct window {
    uint32_t id;
    char title[64];
    int x, y;
    int width, height;
    bool visible;
    bool focused;
    uint8_t* framebuffer;
    struct window* next;
} window_t;

// Process structure
typedef struct process {
    uint32_t pid;
    char name[32];
    uint32_t state;
    uint32_t priority;
    
    // Memory management
    uint64_t stack_pointer;
    uint64_t stack_base;
    uint64_t heap_start;
    uint64_t heap_size;
    
    // Execution context
    uint64_t entry_point;
    uint64_t instruction_pointer;
    uint64_t registers[16];  // General purpose registers
    
    // Process relationships
    uint32_t parent_pid;
    struct process* parent;
    struct process* children;
    struct process* next_sibling;
    
    // Scheduling
    uint64_t cpu_time;
    uint64_t last_scheduled;
    uint32_t time_slice;
    
    // List management
    struct process* next;
    struct process* prev;
    
    // Process flags
    bool is_demon;        // System process
    bool is_kernel_mode;  // Kernel mode process
    bool is_suspended;    // Suspended process
    
    // Creation time
    uint64_t creation_time;
} process_t;

// Function prototypes

// Core kernel functions
void kernel_main(void);
void init_drivers(void);
void display_hell_screen(void);
void play_startup_sound(void);
void start_infernal_shell(void);
void kernel_main_loop(void);
void kernel_panic(const char* message);
void kernel_shutdown(void);

// Graphics functions
void draw_hell_logo(void);
void draw_flame_border(void);
void clear_screen(uint8_t color);
void draw_pixel(int x, int y, uint8_t color);
void draw_text(const char* text, int x, int y, uint8_t color);
void draw_char(char c, int x, int y, uint8_t color);
void draw_rectangle(int x, int y, int width, int height, uint8_t color);

// Audio functions
void play_error_sound(void);
void play_shutdown_sound(void);
void play_audio_sequence(audio_note_t* sequence, int channels);
void play_note(int channel, uint16_t frequency, uint8_t waveform, uint8_t volume);

// System functions
void halt_cpu(void);
void yield_cpu(void);
void disable_interrupts(void);
void enable_interrupts(void);
bool check_shutdown_request(void);

// Process management
void stop_all_processes(void);
void shutdown_drivers(void);
void start_shell_process(window_t* window);

// Window management
window_t* create_window(const char* title, int x, int y, int width, int height);
void init_pandemonium_wm(void);
void update_pandemonium_wm(void);

// Shell functions
void process_shell_input(char c);

// System initialization
void init_memory_manager(void);
void init_interrupt_system(void);
void init_graphics_system(void);
void init_audio_system(void);
void init_process_manager(void);
int init_hell_graphics_driver(void);
int init_hell_audio_driver(void);
int init_network_driver(void);

// System updates
void process_interrupts(void);
void update_audio_system(void);
void process_network_packets(void);

// Global kernel state
extern kernel_state_t kernel_state;

// Inline assembly helpers
static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outw(uint16_t port, uint16_t value) {
    __asm__ volatile ("outw %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint16_t inw(uint16_t port) {
    uint16_t ret;
    __asm__ volatile ("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outl(uint16_t port, uint32_t value) {
    __asm__ volatile ("outl %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint32_t inl(uint16_t port) {
    uint32_t ret;
    __asm__ volatile ("inl %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// Memory management macros
#define ALIGN_UP(addr, align) (((addr) + (align) - 1) & ~((align) - 1))
#define ALIGN_DOWN(addr, align) ((addr) & ~((align) - 1))

// Bit manipulation macros
#define BIT(n) (1UL << (n))
#define SET_BIT(var, bit) ((var) |= BIT(bit))
#define CLEAR_BIT(var, bit) ((var) &= ~BIT(bit))
#define TOGGLE_BIT(var, bit) ((var) ^= BIT(bit))
#define CHECK_BIT(var, bit) (((var) >> (bit)) & 1)

// Error codes
#define HELL_SUCCESS        0
#define HELL_ERROR_GENERAL  1
#define HELL_ERROR_MEMORY   2
#define HELL_ERROR_DEVICE   3
#define HELL_ERROR_NETWORK  4
#define HELL_ERROR_AUDIO    5
#define HELL_ERROR_GRAPHICS 6

#endif // KERNEL_H 