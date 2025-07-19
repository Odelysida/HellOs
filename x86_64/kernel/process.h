/*
 * HellOS Process Management Header
 * Process system definitions and structures
 */

#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>
#include <stdbool.h>

// Forward declarations
typedef struct process process_t;
typedef struct process_stats_s process_stats_t;

// Process priority levels
typedef enum {
    PRIORITY_OVERLORD = 0,    // Highest priority (system processes)
    PRIORITY_DEMON = 1,       // High priority (important processes)
    PRIORITY_SOUL = 2,        // Normal priority (user processes)
    PRIORITY_DAMNED = 3       // Lowest priority (background processes)
} process_priority_t;

// Process management functions
void init_process_manager(void);
process_t* create_process(const char* name, uint64_t entry_point, process_priority_t priority, bool is_demon);
void terminate_process(uint32_t pid);
process_t* find_process_by_pid(uint32_t pid);

// Process scheduling
void schedule_next_process(void);
void yield_process(void);
void suspend_process(uint32_t pid);
void resume_process(uint32_t pid);
void process_scheduler(void);

// Process context management
void save_process_context(process_t* process);
void load_process_context(process_t* process);

// Process queue management
void add_to_ready_queue(process_t* process);
void remove_from_ready_queue(process_t* process);

// Process information
process_t* get_current_process(void);
process_t* get_process_list(void);
process_stats_t* get_process_stats(void);
uint32_t get_process_count(void);
uint64_t get_system_time(void);

// Process listing
void list_processes(void);

#endif // PROCESS_H 