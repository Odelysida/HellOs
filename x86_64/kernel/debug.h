/*
 * HellOS Global Debug and Logging System
 * Provides unified debugging, logging, and error tracking across the entire OS
 */

#ifndef DEBUG_H
#define DEBUG_H

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

// Debug levels
typedef enum {
    DEBUG_LEVEL_TRACE = 0,
    DEBUG_LEVEL_DEBUG = 1,
    DEBUG_LEVEL_INFO = 2,
    DEBUG_LEVEL_WARN = 3,
    DEBUG_LEVEL_ERROR = 4,
    DEBUG_LEVEL_FATAL = 5
} debug_level_t;

// Debug subsystems
typedef enum {
    DEBUG_SUBSYSTEM_BOOT = 0,
    DEBUG_SUBSYSTEM_KERNEL = 1,
    DEBUG_SUBSYSTEM_MEMORY = 2,
    DEBUG_SUBSYSTEM_INTERRUPTS = 3,
    DEBUG_SUBSYSTEM_PROCESS = 4,
    DEBUG_SUBSYSTEM_GRAPHICS = 5,
    DEBUG_SUBSYSTEM_AUDIO = 6,
    DEBUG_SUBSYSTEM_NETWORK = 7,
    DEBUG_SUBSYSTEM_SHELL = 8,
    DEBUG_SUBSYSTEM_DRIVERS = 9,
    DEBUG_SUBSYSTEM_MAX = 10
} debug_subsystem_t;

// Debug output targets
typedef enum {
    DEBUG_OUTPUT_VGA = 0x01,
    DEBUG_OUTPUT_SERIAL = 0x02,
    DEBUG_OUTPUT_MEMORY = 0x04,
    DEBUG_OUTPUT_ALL = 0xFF
} debug_output_t;

// Debug configuration
typedef struct {
    debug_level_t min_level;
    debug_output_t output_mask;
    uint32_t subsystem_mask;
    uint32_t buffer_size;
    uint32_t max_entries;
    uint8_t color_enabled;
    uint8_t timestamp_enabled;
    uint8_t subsystem_names_enabled;
} debug_config_t;

// Debug log entry
typedef struct {
    uint32_t timestamp;
    debug_level_t level;
    debug_subsystem_t subsystem;
    char message[256];
} debug_log_entry_t;

// Debug buffer
typedef struct {
    debug_log_entry_t* entries;
    uint32_t head;
    uint32_t tail;
    uint32_t count;
    uint32_t max_entries;
    uint32_t dropped_count;
} debug_buffer_t;

// Global debug state
typedef struct {
    debug_config_t config;
    debug_buffer_t buffer;
    uint32_t boot_time;
    uint32_t initialized;
    uint32_t panic_count;
    uint32_t error_count;
    uint32_t warning_count;
} debug_state_t;

// Debug system initialization
void debug_init(void);
void debug_early_init(void);
void debug_configure(const debug_config_t* config);

// Core logging functions
void debug_log(debug_level_t level, debug_subsystem_t subsystem, const char* format, ...);
void debug_log_raw(debug_level_t level, debug_subsystem_t subsystem, const char* message);
void debug_vlog(debug_level_t level, debug_subsystem_t subsystem, const char* format, va_list args);

// Convenience macros for different log levels
#define DEBUG_TRACE(subsystem, format, ...) debug_log(DEBUG_LEVEL_TRACE, subsystem, format, ##__VA_ARGS__)
#define DEBUG_DEBUG(subsystem, format, ...) debug_log(DEBUG_LEVEL_DEBUG, subsystem, format, ##__VA_ARGS__)
#define DEBUG_INFO(subsystem, format, ...) debug_log(DEBUG_LEVEL_INFO, subsystem, format, ##__VA_ARGS__)
#define DEBUG_WARN(subsystem, format, ...) debug_log(DEBUG_LEVEL_WARN, subsystem, format, ##__VA_ARGS__)
#define DEBUG_ERROR(subsystem, format, ...) debug_log(DEBUG_LEVEL_ERROR, subsystem, format, ##__VA_ARGS__)
#define DEBUG_FATAL(subsystem, format, ...) debug_log(DEBUG_LEVEL_FATAL, subsystem, format, ##__VA_ARGS__)

// Subsystem-specific macros
#define DEBUG_BOOT(level, format, ...) debug_log(level, DEBUG_SUBSYSTEM_BOOT, format, ##__VA_ARGS__)
#define DEBUG_KERNEL(level, format, ...) debug_log(level, DEBUG_SUBSYSTEM_KERNEL, format, ##__VA_ARGS__)
#define DEBUG_MEMORY(level, format, ...) debug_log(level, DEBUG_SUBSYSTEM_MEMORY, format, ##__VA_ARGS__)
#define DEBUG_INTERRUPTS(level, format, ...) debug_log(level, DEBUG_SUBSYSTEM_INTERRUPTS, format, ##__VA_ARGS__)
#define DEBUG_PROCESS(level, format, ...) debug_log(level, DEBUG_SUBSYSTEM_PROCESS, format, ##__VA_ARGS__)
#define DEBUG_GRAPHICS(level, format, ...) debug_log(level, DEBUG_SUBSYSTEM_GRAPHICS, format, ##__VA_ARGS__)
#define DEBUG_AUDIO(level, format, ...) debug_log(level, DEBUG_SUBSYSTEM_AUDIO, format, ##__VA_ARGS__)
#define DEBUG_NETWORK(level, format, ...) debug_log(level, DEBUG_SUBSYSTEM_NETWORK, format, ##__VA_ARGS__)
#define DEBUG_SHELL(level, format, ...) debug_log(level, DEBUG_SUBSYSTEM_SHELL, format, ##__VA_ARGS__)
#define DEBUG_DRIVERS(level, format, ...) debug_log(level, DEBUG_SUBSYSTEM_DRIVERS, format, ##__VA_ARGS__)

// Memory dump functions
void debug_dump_memory(const void* ptr, uint32_t size, const char* label);
void debug_dump_registers(void);
void debug_dump_stack(uint32_t depth);

// Buffer management
void debug_buffer_clear(void);
void debug_buffer_dump(void);
uint32_t debug_buffer_get_count(void);
debug_log_entry_t* debug_buffer_get_entry(uint32_t index);

// Output functions
void debug_output_vga(const char* message, debug_level_t level);
void debug_output_serial(const char* message);
void debug_output_memory(const char* message);

// Panic and assertion functions
void debug_panic(const char* message) __attribute__((noreturn));
void debug_assert(int condition, const char* message);

// Performance monitoring
void debug_perf_start(const char* name);
void debug_perf_end(const char* name);
void debug_perf_report(void);

// Debug shell commands
void debug_shell_help(void);
void debug_shell_level(debug_level_t level);
void debug_shell_subsystem(debug_subsystem_t subsystem, uint8_t enabled);
void debug_shell_buffer_dump(void);
void debug_shell_stats(void);

// Utility functions
const char* debug_level_to_string(debug_level_t level);
const char* debug_subsystem_to_string(debug_subsystem_t subsystem);
uint32_t debug_get_timestamp(void);
void debug_format_message(char* buffer, size_t buffer_size, debug_level_t level, debug_subsystem_t subsystem, const char* message);

// Color definitions for VGA output
#define DEBUG_COLOR_TRACE   0x08    // Dark gray
#define DEBUG_COLOR_DEBUG   0x07    // Light gray
#define DEBUG_COLOR_INFO    0x0F    // White
#define DEBUG_COLOR_WARN    0x0E    // Yellow
#define DEBUG_COLOR_ERROR   0x0C    // Red
#define DEBUG_COLOR_FATAL   0x4C    // Red background

// Serial port definitions
#define DEBUG_SERIAL_PORT   0x3F8   // COM1
#define DEBUG_SERIAL_BAUD   115200

// Memory buffer size
#define DEBUG_MEMORY_BUFFER_SIZE    (64 * 1024)  // 64KB debug buffer

// Boot-time debugging (before full system init)
void debug_boot_print(const char* message);
void debug_boot_hex(uint32_t value);
void debug_boot_init_serial(void);

#endif // DEBUG_H 