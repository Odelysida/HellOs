/*
 * HellOS Debug Log Viewer
 * Utility for analyzing debug output from HellOS
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define MAX_LINE_LENGTH 1024
#define MAX_LINES 10000

typedef struct {
    char* line;
    int level;
    char* subsystem;
    char* timestamp;
    char* message;
} log_entry_t;

typedef struct {
    log_entry_t entries[MAX_LINES];
    int count;
} log_buffer_t;

// Color codes for different log levels
#define COLOR_RESET     "\033[0m"
#define COLOR_TRACE     "\033[90m"  // Dark gray
#define COLOR_DEBUG     "\033[37m"  // Light gray
#define COLOR_INFO      "\033[37m"  // White
#define COLOR_WARN      "\033[33m"  // Yellow
#define COLOR_ERROR     "\033[31m"  // Red
#define COLOR_FATAL     "\033[41m"  // Red background

static log_buffer_t g_log_buffer = {0};

// Parse log level from string
int parse_log_level(const char* level_str) {
    if (strncmp(level_str, "TRACE", 5) == 0) return 0;
    if (strncmp(level_str, "DEBUG", 5) == 0) return 1;
    if (strncmp(level_str, "INFO", 4) == 0) return 2;
    if (strncmp(level_str, "WARN", 4) == 0) return 3;
    if (strncmp(level_str, "ERROR", 5) == 0) return 4;
    if (strncmp(level_str, "FATAL", 5) == 0) return 5;
    return 2; // Default to INFO
}

// Get color for log level
const char* get_level_color(int level) {
    switch (level) {
        case 0: return COLOR_TRACE;
        case 1: return COLOR_DEBUG;
        case 2: return COLOR_INFO;
        case 3: return COLOR_WARN;
        case 4: return COLOR_ERROR;
        case 5: return COLOR_FATAL;
        default: return COLOR_INFO;
    }
}

// Parse a single log line
int parse_log_line(const char* line, log_entry_t* entry) {
    char* line_copy = strdup(line);
    char* ptr = line_copy;
    
    // Skip whitespace
    while (*ptr == ' ' || *ptr == '\t') ptr++;
    
    // Parse timestamp [timestamp]
    if (*ptr == '[') {
        ptr++; // Skip '['
        char* timestamp_end = strchr(ptr, ']');
        if (timestamp_end) {
            *timestamp_end = '\0';
            entry->timestamp = strdup(ptr);
            ptr = timestamp_end + 1;
        }
    }
    
    // Skip whitespace
    while (*ptr == ' ' || *ptr == '\t') ptr++;
    
    // Parse level [LEVEL]
    if (*ptr == '[') {
        ptr++; // Skip '['
        char* level_end = strchr(ptr, ']');
        if (level_end) {
            *level_end = '\0';
            entry->level = parse_log_level(ptr);
            ptr = level_end + 1;
        }
    }
    
    // Skip whitespace
    while (*ptr == ' ' || *ptr == '\t') ptr++;
    
    // Parse subsystem [SUBSYSTEM]
    if (*ptr == '[') {
        ptr++; // Skip '['
        char* subsystem_end = strchr(ptr, ']');
        if (subsystem_end) {
            *subsystem_end = '\0';
            entry->subsystem = strdup(ptr);
            ptr = subsystem_end + 1;
        }
    }
    
    // Skip whitespace
    while (*ptr == ' ' || *ptr == '\t') ptr++;
    
    // Rest is the message
    entry->message = strdup(ptr);
    entry->line = strdup(line);
    
    free(line_copy);
    return 1;
}

// Load log file
int load_log_file(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open log file");
        return 0;
    }
    
    char line[MAX_LINE_LENGTH];
    g_log_buffer.count = 0;
    
    while (fgets(line, sizeof(line), file) && g_log_buffer.count < MAX_LINES) {
        // Remove newline
        char* newline = strchr(line, '\n');
        if (newline) *newline = '\0';
        
        // Skip empty lines
        if (strlen(line) == 0) continue;
        
        // Parse line
        if (parse_log_line(line, &g_log_buffer.entries[g_log_buffer.count])) {
            g_log_buffer.count++;
        }
    }
    
    fclose(file);
    printf("Loaded %d log entries from %s\n", g_log_buffer.count, filename);
    return 1;
}

// Display log entries with filtering
void display_logs(int min_level, const char* subsystem_filter, int use_colors) {
    printf("\n=== HellOS Debug Log Analysis ===\n");
    printf("Showing %d entries", g_log_buffer.count);
    if (min_level > 0) {
        const char* levels[] = {"TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"};
        printf(" (level >= %s)", levels[min_level]);
    }
    if (subsystem_filter) {
        printf(" (subsystem: %s)", subsystem_filter);
    }
    printf("\n\n");
    
    int displayed = 0;
    for (int i = 0; i < g_log_buffer.count; i++) {
        log_entry_t* entry = &g_log_buffer.entries[i];
        
        // Apply level filter
        if (entry->level < min_level) continue;
        
        // Apply subsystem filter
        if (subsystem_filter && entry->subsystem && 
            strstr(entry->subsystem, subsystem_filter) == NULL) {
            continue;
        }
        
        // Display entry
        if (use_colors) {
            printf("%s", get_level_color(entry->level));
        }
        
        if (entry->timestamp) {
            printf("[%s] ", entry->timestamp);
        }
        
        const char* levels[] = {"TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"};
        printf("[%s] ", levels[entry->level]);
        
        if (entry->subsystem) {
            printf("[%s] ", entry->subsystem);
        }
        
        if (entry->message) {
            printf("%s", entry->message);
        }
        
        if (use_colors) {
            printf("%s", COLOR_RESET);
        }
        
        printf("\n");
        displayed++;
    }
    
    printf("\nDisplayed %d entries\n", displayed);
}

// Show statistics
void show_statistics(void) {
    int level_counts[6] = {0};
    int subsystem_counts[10] = {0};
    const char* subsystems[] = {
        "BOOT", "KERNEL", "MEMORY", "INTERRUPTS", "PROCESS",
        "GRAPHICS", "AUDIO", "NETWORK", "SHELL", "DRIVERS"
    };
    
    for (int i = 0; i < g_log_buffer.count; i++) {
        log_entry_t* entry = &g_log_buffer.entries[i];
        
        // Count by level
        if (entry->level >= 0 && entry->level < 6) {
            level_counts[entry->level]++;
        }
        
        // Count by subsystem
        if (entry->subsystem) {
            for (int j = 0; j < 10; j++) {
                if (strstr(entry->subsystem, subsystems[j])) {
                    subsystem_counts[j]++;
                    break;
                }
            }
        }
    }
    
    printf("\n=== Log Statistics ===\n");
    printf("Total entries: %d\n\n", g_log_buffer.count);
    
    printf("By Level:\n");
    const char* levels[] = {"TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"};
    for (int i = 0; i < 6; i++) {
        if (level_counts[i] > 0) {
            printf("  %s: %d\n", levels[i], level_counts[i]);
        }
    }
    
    printf("\nBy Subsystem:\n");
    for (int i = 0; i < 10; i++) {
        if (subsystem_counts[i] > 0) {
            printf("  %s: %d\n", subsystems[i], subsystem_counts[i]);
        }
    }
}

// Show help
void show_help(void) {
    printf("HellOS Debug Log Viewer\n");
    printf("Usage: debug_viewer [options] <log_file>\n\n");
    printf("Options:\n");
    printf("  -l <level>     Minimum log level (0-5: TRACE, DEBUG, INFO, WARN, ERROR, FATAL)\n");
    printf("  -s <subsystem> Filter by subsystem (BOOT, KERNEL, MEMORY, etc.)\n");
    printf("  -c             Use colors in output\n");
    printf("  -S             Show statistics only\n");
    printf("  -h             Show this help\n\n");
    printf("Examples:\n");
    printf("  debug_viewer serial.log\n");
    printf("  debug_viewer -l 3 -c serial.log    # Show warnings and errors with colors\n");
    printf("  debug_viewer -s BOOT serial.log    # Show only boot messages\n");
    printf("  debug_viewer -S serial.log         # Show statistics only\n");
}

int main(int argc, char* argv[]) {
    int min_level = 0;
    char* subsystem_filter = NULL;
    int use_colors = 0;
    int show_stats_only = 0;
    char* filename = NULL;
    
    // Parse command line arguments
    int opt;
    while ((opt = getopt(argc, argv, "l:s:cSh")) != -1) {
        switch (opt) {
            case 'l':
                min_level = atoi(optarg);
                if (min_level < 0 || min_level > 5) {
                    fprintf(stderr, "Invalid log level: %d (must be 0-5)\n", min_level);
                    return 1;
                }
                break;
            case 's':
                subsystem_filter = optarg;
                break;
            case 'c':
                use_colors = 1;
                break;
            case 'S':
                show_stats_only = 1;
                break;
            case 'h':
                show_help();
                return 0;
            default:
                show_help();
                return 1;
        }
    }
    
    // Get filename
    if (optind >= argc) {
        fprintf(stderr, "Error: No log file specified\n\n");
        show_help();
        return 1;
    }
    filename = argv[optind];
    
    // Load log file
    if (!load_log_file(filename)) {
        return 1;
    }
    
    // Show statistics or display logs
    if (show_stats_only) {
        show_statistics();
    } else {
        display_logs(min_level, subsystem_filter, use_colors);
        printf("\nUse -S option to show statistics\n");
    }
    
    return 0;
} 