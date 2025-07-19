/*
 * HellOS Global Debug and Logging System Implementation
 * Provides unified debugging, logging, and error tracking across the entire OS
 */

#include "debug.h"
#include "memory_layout.h"
#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

// Port I/O functions (inline assembly)
static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t value;
    __asm__ volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

// Forward declarations
static int debug_sprintf(char* buffer, const char* format, ...);
static int debug_vsprintf(char* buffer, const char* format, va_list args);
static int debug_int_to_string(char* buffer, int value);
static int debug_hex_to_string(char* buffer, unsigned int value);

// Global debug state
static debug_state_t g_debug_state = {0};
static debug_log_entry_t g_debug_entries[1024];
static char g_debug_memory_buffer[DEBUG_MEMORY_BUFFER_SIZE];
static uint32_t g_debug_memory_offset = 0;

// Subsystem names
static const char* g_subsystem_names[] = {
    "BOOT", "KERNEL", "MEMORY", "INTERRUPTS", "PROCESS",
    "GRAPHICS", "AUDIO", "NETWORK", "SHELL", "DRIVERS"
};

// Level names
static const char* g_level_names[] = {
    "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
};

// Level colors
static const uint8_t g_level_colors[] = {
    DEBUG_COLOR_TRACE, DEBUG_COLOR_DEBUG, DEBUG_COLOR_INFO,
    DEBUG_COLOR_WARN, DEBUG_COLOR_ERROR, DEBUG_COLOR_FATAL
};

// Simple string functions (since we can't use standard library)
static int debug_strlen(const char* str) {
    int len = 0;
    while (str[len]) len++;
    return len;
}

static char* debug_strcpy(char* dest, const char* src) {
    int i = 0;
    while (src[i]) {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
    return dest;
}

static char* debug_strcat(char* dest, const char* src) {
    int dest_len = debug_strlen(dest);
    int i = 0;
    while (src[i]) {
        dest[dest_len + i] = src[i];
        i++;
    }
    dest[dest_len + i] = '\0';
    return dest;
}

// Simple sprintf implementation for debug messages
static int debug_sprintf(char* buffer, const char* format, ...) {
    va_list args;
    va_start(args, format);
    int result = debug_vsprintf(buffer, format, args);
    va_end(args);
    return result;
}

static int debug_vsprintf(char* buffer, const char* format, va_list args) {
    int pos = 0;
    int i = 0;
    
    while (format[i]) {
        if (format[i] == '%' && format[i + 1]) {
            i++; // Skip '%'
            switch (format[i]) {
                case 'd': {
                    int value = va_arg(args, int);
                    pos += debug_int_to_string(buffer + pos, value);
                    break;
                }
                case 'x': {
                    unsigned int value = va_arg(args, unsigned int);
                    pos += debug_hex_to_string(buffer + pos, value);
                    break;
                }
                case 's': {
                    char* str = va_arg(args, char*);
                    if (str) {
                        while (*str) {
                            buffer[pos++] = *str++;
                        }
                    }
                    break;
                }
                case 'c': {
                    char c = (char)va_arg(args, int);
                    buffer[pos++] = c;
                    break;
                }
                case '%': {
                    buffer[pos++] = '%';
                    break;
                }
                default:
                    buffer[pos++] = format[i];
                    break;
            }
        } else {
            buffer[pos++] = format[i];
        }
        i++;
    }
    buffer[pos] = '\0';
    return pos;
}

static int debug_int_to_string(char* buffer, int value) {
    if (value == 0) {
        buffer[0] = '0';
        buffer[1] = '\0';
        return 1;
    }
    
    int pos = 0;
    int negative = 0;
    
    if (value < 0) {
        negative = 1;
        value = -value;
    }
    
    // Convert digits in reverse order
    char temp[16];
    int temp_pos = 0;
    while (value > 0) {
        temp[temp_pos++] = '0' + (value % 10);
        value /= 10;
    }
    
    if (negative) {
        buffer[pos++] = '-';
    }
    
    // Reverse the digits
    for (int i = temp_pos - 1; i >= 0; i--) {
        buffer[pos++] = temp[i];
    }
    
    buffer[pos] = '\0';
    return pos;
}

static int debug_hex_to_string(char* buffer, unsigned int value) {
    const char hex_chars[] = "0123456789ABCDEF";
    int pos = 0;
    
    buffer[pos++] = '0';
    buffer[pos++] = 'x';
    
    if (value == 0) {
        buffer[pos++] = '0';
        buffer[pos] = '\0';
        return pos;
    }
    
    // Convert hex digits in reverse order
    char temp[16];
    int temp_pos = 0;
    while (value > 0) {
        temp[temp_pos++] = hex_chars[value & 0xF];
        value >>= 4;
    }
    
    // Reverse the digits
    for (int i = temp_pos - 1; i >= 0; i--) {
        buffer[pos++] = temp[i];
    }
    
    buffer[pos] = '\0';
    return pos;
}

// Early initialization (before memory manager)
void debug_early_init(void) {
    // Initialize serial port for early debugging
    debug_boot_init_serial();
    
    // Clear debug state
    g_debug_state.config.min_level = DEBUG_LEVEL_INFO;
    g_debug_state.config.output_mask = DEBUG_OUTPUT_SERIAL | DEBUG_OUTPUT_VGA;
    g_debug_state.config.subsystem_mask = 0xFFFFFFFF; // All subsystems
    g_debug_state.config.buffer_size = DEBUG_MEMORY_BUFFER_SIZE;
    g_debug_state.config.max_entries = 1024;
    g_debug_state.config.color_enabled = 1;
    g_debug_state.config.timestamp_enabled = 1;
    g_debug_state.config.subsystem_names_enabled = 1;
    
    // Initialize buffer
    g_debug_state.buffer.entries = g_debug_entries;
    g_debug_state.buffer.head = 0;
    g_debug_state.buffer.tail = 0;
    g_debug_state.buffer.count = 0;
    g_debug_state.buffer.max_entries = 1024;
    g_debug_state.buffer.dropped_count = 0;
    
    g_debug_state.boot_time = 0;
    g_debug_state.initialized = 1;
    g_debug_state.panic_count = 0;
    g_debug_state.error_count = 0;
    g_debug_state.warning_count = 0;
    
    DEBUG_BOOT(DEBUG_LEVEL_INFO, "Debug system early initialization complete");
}

// Full initialization (after memory manager)
void debug_init(void) {
    if (!g_debug_state.initialized) {
        debug_early_init();
    }
    
    DEBUG_KERNEL(DEBUG_LEVEL_INFO, "Debug system full initialization complete");
}

// Configure debug system
void debug_configure(const debug_config_t* config) {
    if (config) {
        g_debug_state.config = *config;
        DEBUG_KERNEL(DEBUG_LEVEL_INFO, "Debug system reconfigured");
    }
}

// Core logging function
void debug_log(debug_level_t level, debug_subsystem_t subsystem, const char* format, ...) {
    if (!g_debug_state.initialized) {
        return;
    }
    
    // Check if this log level and subsystem are enabled
    if (level < g_debug_state.config.min_level) {
        return;
    }
    
    if (!(g_debug_state.config.subsystem_mask & (1 << subsystem))) {
        return;
    }
    
    va_list args;
    va_start(args, format);
    debug_vlog(level, subsystem, format, args);
    va_end(args);
}

// Raw logging function
void debug_log_raw(debug_level_t level, debug_subsystem_t subsystem, const char* message) {
    if (!g_debug_state.initialized) {
        return;
    }
    
    // Check if this log level and subsystem are enabled
    if (level < g_debug_state.config.min_level) {
        return;
    }
    
    if (!(g_debug_state.config.subsystem_mask & (1 << subsystem))) {
        return;
    }
    
    // Update statistics
    switch (level) {
        case DEBUG_LEVEL_WARN:
            g_debug_state.warning_count++;
            break;
        case DEBUG_LEVEL_ERROR:
            g_debug_state.error_count++;
            break;
        case DEBUG_LEVEL_FATAL:
            g_debug_state.panic_count++;
            break;
        default:
            break;
    }
    
    // Add to buffer
    if (g_debug_state.buffer.count < g_debug_state.buffer.max_entries) {
        debug_log_entry_t* entry = &g_debug_state.buffer.entries[g_debug_state.buffer.tail];
        entry->timestamp = debug_get_timestamp();
        entry->level = level;
        entry->subsystem = subsystem;
        debug_strcpy(entry->message, message);
        
        g_debug_state.buffer.tail = (g_debug_state.buffer.tail + 1) % g_debug_state.buffer.max_entries;
        g_debug_state.buffer.count++;
    } else {
        g_debug_state.buffer.dropped_count++;
    }
    
    // Format and output message
    char formatted_message[512];
    debug_format_message(formatted_message, sizeof(formatted_message), level, subsystem, message);
    
    // Output to enabled targets
    if (g_debug_state.config.output_mask & DEBUG_OUTPUT_VGA) {
        debug_output_vga(formatted_message, level);
    }
    
    if (g_debug_state.config.output_mask & DEBUG_OUTPUT_SERIAL) {
        debug_output_serial(formatted_message);
    }
    
    if (g_debug_state.config.output_mask & DEBUG_OUTPUT_MEMORY) {
        debug_output_memory(formatted_message);
    }
}

// Variable argument logging
void debug_vlog(debug_level_t level, debug_subsystem_t subsystem, const char* format, va_list args) {
    char message[256];
    debug_vsprintf(message, format, args);
    debug_log_raw(level, subsystem, message);
}

// Format message with timestamp and subsystem
void debug_format_message(char* buffer, size_t buffer_size, debug_level_t level, debug_subsystem_t subsystem, const char* message) {
    buffer[0] = '\0';
    
    // Add timestamp if enabled
    if (g_debug_state.config.timestamp_enabled) {
        char timestamp_str[16];
        debug_sprintf(timestamp_str, "[%d] ", debug_get_timestamp());
        debug_strcat(buffer, timestamp_str);
    }
    
    // Add level
    debug_strcat(buffer, "[");
    debug_strcat(buffer, debug_level_to_string(level));
    debug_strcat(buffer, "] ");
    
    // Add subsystem if enabled
    if (g_debug_state.config.subsystem_names_enabled) {
        debug_strcat(buffer, "[");
        debug_strcat(buffer, debug_subsystem_to_string(subsystem));
        debug_strcat(buffer, "] ");
    }
    
    // Add message
    debug_strcat(buffer, message);
    debug_strcat(buffer, "\n");
}

// VGA output
void debug_output_vga(const char* message, debug_level_t level) {
    volatile uint16_t* vga_buffer = (volatile uint16_t*)VGA_TEXT_BUFFER;
    static int vga_row = 0;
    static int vga_col = 0;
    
    uint8_t color = g_debug_state.config.color_enabled ? g_level_colors[level] : 0x07;
    
    int i = 0;
    while (message[i]) {
        if (message[i] == '\n') {
            vga_col = 0;
            vga_row++;
            if (vga_row >= VGA_TEXT_HEIGHT) {
                // Scroll screen
                for (int row = 0; row < VGA_TEXT_HEIGHT - 1; row++) {
                    for (int col = 0; col < VGA_TEXT_WIDTH; col++) {
                        vga_buffer[row * VGA_TEXT_WIDTH + col] = vga_buffer[(row + 1) * VGA_TEXT_WIDTH + col];
                    }
                }
                // Clear last row
                for (int col = 0; col < VGA_TEXT_WIDTH; col++) {
                    vga_buffer[(VGA_TEXT_HEIGHT - 1) * VGA_TEXT_WIDTH + col] = (color << 8) | ' ';
                }
                vga_row = VGA_TEXT_HEIGHT - 1;
            }
        } else {
            if (vga_col < VGA_TEXT_WIDTH) {
                vga_buffer[vga_row * VGA_TEXT_WIDTH + vga_col] = (color << 8) | message[i];
                vga_col++;
            }
        }
        i++;
    }
}

// Serial output
void debug_output_serial(const char* message) {
    int i = 0;
    while (message[i]) {
        // Wait for transmit buffer to be empty
        while (!(inb(DEBUG_SERIAL_PORT + 5) & 0x20));
        outb(DEBUG_SERIAL_PORT, message[i]);
        i++;
    }
}

// Memory buffer output
void debug_output_memory(const char* message) {
    int len = debug_strlen(message);
    if (g_debug_memory_offset + len < DEBUG_MEMORY_BUFFER_SIZE) {
        debug_strcpy(&g_debug_memory_buffer[g_debug_memory_offset], message);
        g_debug_memory_offset += len;
    }
}

// Utility functions
const char* debug_level_to_string(debug_level_t level) {
    if (level < sizeof(g_level_names) / sizeof(g_level_names[0])) {
        return g_level_names[level];
    }
    return "UNKNOWN";
}

const char* debug_subsystem_to_string(debug_subsystem_t subsystem) {
    if (subsystem < sizeof(g_subsystem_names) / sizeof(g_subsystem_names[0])) {
        return g_subsystem_names[subsystem];
    }
    return "UNKNOWN";
}

// Get timestamp (simple tick counter for now)
uint32_t debug_get_timestamp(void) {
    // TODO: Implement proper timestamp based on system timer
    static uint32_t tick_counter = 0;
    return tick_counter++;
}

// Panic function
void debug_panic(const char* message) {
    g_debug_state.panic_count++;
    DEBUG_FATAL(DEBUG_SUBSYSTEM_KERNEL, "PANIC: %s", message);
    
    // Disable interrupts
    __asm__ volatile("cli");
    
    // Halt system
    while (1) {
        __asm__ volatile("hlt");
    }
}

// Assert function
void debug_assert(int condition, const char* message) {
    if (!condition) {
        debug_panic(message);
    }
}

// Boot-time functions
void debug_boot_init_serial(void) {
    // Initialize COM1 for debug output
    outb(DEBUG_SERIAL_PORT + 1, 0x00);    // Disable all interrupts
    outb(DEBUG_SERIAL_PORT + 3, 0x80);    // Enable DLAB (set baud rate divisor)
    outb(DEBUG_SERIAL_PORT + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
    outb(DEBUG_SERIAL_PORT + 1, 0x00);    //                  (hi byte)
    outb(DEBUG_SERIAL_PORT + 3, 0x03);    // 8 bits, no parity, one stop bit
    outb(DEBUG_SERIAL_PORT + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
    outb(DEBUG_SERIAL_PORT + 4, 0x0B);    // IRQs enabled, RTS/DSR set
}

void debug_boot_print(const char* message) {
    debug_output_serial(message);
}

void debug_boot_hex(uint32_t value) {
    char hex_str[16];
    debug_hex_to_string(hex_str, value);
    debug_boot_print(hex_str);
} 