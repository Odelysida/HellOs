/*
 * HellOS Kernel - The Infernal Core
 * Main kernel entry point and core functionality
 */

#include "kernel.h"
#include "debug.h"
#include "memory.h"
#include "interrupts.h"
#include "graphics.h"
#include "audio.h"

// Global kernel state
kernel_state_t kernel_state;

// External symbols from bootloader
extern uint64_t framebuffer_base;

/*
 * Main kernel entry point
 * Called from bootloader after UEFI exit
 */
void kernel_main(void) {
    // Initialize debug system first for early debugging
    debug_early_init();
    
    DEBUG_KERNEL(DEBUG_LEVEL_INFO, "HellOS kernel starting up...");
    
    // Initialize kernel state
    kernel_state.boot_time = 0; // TODO: Get actual time
    kernel_state.memory_size = 0; // TODO: Get from memory map
    kernel_state.status = KERNEL_STATUS_INITIALIZING;
    
    DEBUG_KERNEL(DEBUG_LEVEL_INFO, "Initializing core subsystems...");
    
    // Initialize core subsystems
    init_memory_manager();
    DEBUG_KERNEL(DEBUG_LEVEL_INFO, "Memory manager initialized");
    
    init_interrupt_system();
    DEBUG_KERNEL(DEBUG_LEVEL_INFO, "Interrupt system initialized");
    
    init_process_manager();
    DEBUG_KERNEL(DEBUG_LEVEL_INFO, "Process manager initialized");
    
    init_graphics_system();
    DEBUG_KERNEL(DEBUG_LEVEL_INFO, "Graphics system initialized");
    
    init_audio_system();
    DEBUG_KERNEL(DEBUG_LEVEL_INFO, "Audio system initialized");
    
    // Full debug system initialization (after memory manager)
    debug_init();
    
    // Display hellish boot screen
    display_hell_screen();
    
    // Play demonic startup sound
    play_startup_sound();
    
    // Initialize device drivers
    init_drivers();
    
    // Start the infernal shell
    start_infernal_shell();
    
    // Main kernel loop
    kernel_main_loop();
}

/*
 * Initialize all device drivers
 */
void init_drivers(void) {
    DEBUG_DRIVERS(DEBUG_LEVEL_INFO, "Initializing device drivers...");
    
    // Graphics driver
    DEBUG_DRIVERS(DEBUG_LEVEL_INFO, "Initializing graphics driver...");
    if (init_hell_graphics_driver() != 0) {
        DEBUG_DRIVERS(DEBUG_LEVEL_ERROR, "Failed to initialize graphics driver");
        kernel_panic("Failed to initialize graphics driver");
    }
    DEBUG_DRIVERS(DEBUG_LEVEL_INFO, "Graphics driver initialized successfully");
    
    // Audio driver
    DEBUG_DRIVERS(DEBUG_LEVEL_INFO, "Initializing audio driver...");
    if (init_hell_audio_driver() != 0) {
        DEBUG_DRIVERS(DEBUG_LEVEL_ERROR, "Failed to initialize audio driver");
        kernel_panic("Failed to initialize audio driver");
    }
    DEBUG_DRIVERS(DEBUG_LEVEL_INFO, "Audio driver initialized successfully");
    
    // Network driver (basic)
    DEBUG_DRIVERS(DEBUG_LEVEL_INFO, "Initializing network driver...");
    if (init_network_driver() != 0) {
        DEBUG_DRIVERS(DEBUG_LEVEL_ERROR, "Failed to initialize network driver");
        kernel_panic("Failed to initialize network driver");
    }
    DEBUG_DRIVERS(DEBUG_LEVEL_INFO, "Network driver initialized successfully");
    
    kernel_state.status = KERNEL_STATUS_DRIVERS_LOADED;
    DEBUG_DRIVERS(DEBUG_LEVEL_INFO, "All device drivers loaded successfully");
}

/*
 * Display the hellish boot screen
 */
void display_hell_screen(void) {
    // Clear screen with dark red background
    clear_screen(COLOR_HELL_RED);
    
    // Draw hellish ASCII art
    draw_hell_logo();
    
    // Display system information
    draw_text("HellOS - The Infernal Operating System", 10, 10, COLOR_FLAME_ORANGE);
    draw_text("Summoning demons...", 10, 30, COLOR_VOID_BLACK);
    draw_text("Initializing torture chambers...", 10, 50, COLOR_DARK_RED);
    draw_text("Loading souls database...", 10, 70, COLOR_FLAME_ORANGE);
    
    // Draw flame effects around the border
    draw_flame_border();
}

/*
 * Play the demonic startup sound
 */
void play_startup_sound(void) {
    // Demonic chord progression
    audio_note_t startup_chord[] = {
        {NOTE_C2, WAVE_SINE, 500},    // Low C
        {NOTE_DS2, WAVE_SQUARE, 500}, // D# (tritone - devil's interval)
        {NOTE_G2, WAVE_SAW, 500},     // G
        {0, 0, 0} // End marker
    };
    
    play_audio_sequence(startup_chord, 3); // Use all 3 channels
}

/*
 * Start the infernal shell
 */
void start_infernal_shell(void) {
    // Initialize window manager
    init_pandemonium_wm();
    
    // Create main shell window
    window_t* shell_window = create_window("Infernal Shell", 100, 100, 480, 300);
    if (!shell_window) {
        kernel_panic("Failed to create shell window");
    }
    
    // Start shell process
    start_shell_process(shell_window);
    
    kernel_state.status = KERNEL_STATUS_SHELL_ACTIVE;
}

/*
 * Main kernel event loop
 */
void kernel_main_loop(void) {
    kernel_state.status = KERNEL_STATUS_RUNNING;
    
    while (kernel_state.status == KERNEL_STATUS_RUNNING) {
        // Process hardware interrupts
        process_interrupts();
        
        // Update window manager
        update_pandemonium_wm();
        
        // Update audio system
        update_audio_system();
        
        // Process network packets
        process_network_packets();
        
        // Yield to other processes (cooperative multitasking)
        yield_cpu();
        
        // Check for system shutdown
        if (check_shutdown_request()) {
            kernel_shutdown();
        }
    }
}

/*
 * Kernel panic - display error and halt
 */
void kernel_panic(const char* message) {
    // Use debug system panic function which handles everything
    debug_panic(message);
}

/*
 * Shutdown the kernel
 */
void kernel_shutdown(void) {
    kernel_state.status = KERNEL_STATUS_SHUTTING_DOWN;
    
    // Stop all processes
    stop_all_processes();
    
    // Shutdown drivers
    shutdown_drivers();
    
    // Display shutdown message
    clear_screen(COLOR_VOID_BLACK);
    draw_text("The infernal realm is closing...", 10, 10, COLOR_HELL_RED);
    draw_text("All souls have been processed.", 10, 30, COLOR_HELL_RED);
    
    // Final shutdown sound
    play_shutdown_sound();
    
    // Halt system
    kernel_state.status = KERNEL_STATUS_HALTED;
    while (1) {
        halt_cpu();
    }
}

/*
 * Draw the HellOS logo
 */
void draw_hell_logo(void) {
    // ASCII art for HellOS logo
    const char* logo[] = {
        "    ██   ██ ███████ ██      ██       ██████  ███████ ",
        "    ██   ██ ██      ██      ██      ██    ██ ██      ",
        "    ███████ █████   ██      ██      ██    ██ ███████ ",
        "    ██   ██ ██      ██      ██      ██    ██      ██ ",
        "    ██   ██ ███████ ███████ ███████  ██████  ███████ ",
        "                                                     ",
        "            The Infernal Operating System            ",
        NULL
    };
    
    int y = 150;
    for (int i = 0; logo[i] != NULL; i++) {
        draw_text(logo[i], 50, y, COLOR_FLAME_ORANGE);
        y += 20;
    }
}

/*
 * Draw flame border effect
 */
void draw_flame_border(void) {
    // Simple flame effect around screen border
    for (int x = 0; x < SCREEN_WIDTH; x++) {
        // Top border
        draw_pixel(x, 0, COLOR_FLAME_ORANGE);
        draw_pixel(x, 1, COLOR_HELL_RED);
        
        // Bottom border
        draw_pixel(x, SCREEN_HEIGHT - 1, COLOR_FLAME_ORANGE);
        draw_pixel(x, SCREEN_HEIGHT - 2, COLOR_HELL_RED);
    }
    
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        // Left border
        draw_pixel(0, y, COLOR_FLAME_ORANGE);
        draw_pixel(1, y, COLOR_HELL_RED);
        
        // Right border
        draw_pixel(SCREEN_WIDTH - 1, y, COLOR_FLAME_ORANGE);
        draw_pixel(SCREEN_WIDTH - 2, y, COLOR_HELL_RED);
    }
}

/*
 * Play error sound
 */
void play_error_sound(void) {
    // Demonic scream simulation
    audio_note_t error_sound[] = {
        {NOTE_C1, WAVE_SAW, 200},     // Low growl
        {NOTE_DS1, WAVE_SQUARE, 200}, // Harsh sound
        {NOTE_G1, WAVE_SINE, 400},    // Sustained note
        {0, 0, 0}
    };
    
    play_audio_sequence(error_sound, 3);
}

/*
 * Play shutdown sound
 */
void play_shutdown_sound(void) {
    // Fading demonic chord
    audio_note_t shutdown_sound[] = {
        {NOTE_G2, WAVE_SINE, 300},
        {NOTE_DS2, WAVE_SQUARE, 300},
        {NOTE_C2, WAVE_SAW, 600},
        {0, 0, 0}
    };
    
    play_audio_sequence(shutdown_sound, 3);
} 