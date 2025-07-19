/*
 * HellOS Process Management System
 * The infernal soul and demon management
 */

#include "kernel.h"
#include "process.h"
#include "memory.h"
#include <stdint.h>

// Process constants
#define MAX_PROCESSES 64
#define PROCESS_NAME_LENGTH 32

// Process states
typedef enum {
    PROCESS_STATE_READY,
    PROCESS_STATE_RUNNING,
    PROCESS_STATE_WAITING,
    PROCESS_STATE_TERMINATED,
    PROCESS_STATE_ZOMBIE
} process_state_t;

// Process management state
static process_t processes[MAX_PROCESSES];
static process_t* process_list = NULL;
static process_t* current_process = NULL;
static process_t* ready_queue = NULL;
static uint32_t next_pid = 1;
static uint32_t process_count = 0;
static bool process_manager_initialized = false;

// Process statistics
struct process_stats_s {
    uint32_t total_processes;
    uint32_t active_processes;
    uint32_t demon_processes;
    uint32_t soul_processes;
    uint32_t zombie_processes;
    uint64_t context_switches;
    uint64_t total_cpu_time;
};

static process_stats_t process_stats = {0};

/*
 * Initialize the process management system
 */
void init_process_manager(void) {
    // Initialize process array
    for (int i = 0; i < MAX_PROCESSES; i++) {
        processes[i].pid = 0;
        processes[i].state = PROCESS_STATE_TERMINATED;
        processes[i].next = NULL;
        processes[i].prev = NULL;
    }
    
    // Create kernel process (PID 0)
    process_t* kernel_process = create_process("kernel_daemon", 0, PRIORITY_OVERLORD, true);
    if (kernel_process) {
        kernel_process->pid = 0;
        kernel_process->state = PROCESS_STATE_RUNNING;
        current_process = kernel_process;
    }
    
    // Initialize statistics
    process_stats.total_processes = 1;
    process_stats.active_processes = 1;
    process_stats.demon_processes = 1;
    process_stats.soul_processes = 0;
    process_stats.zombie_processes = 0;
    process_stats.context_switches = 0;
    process_stats.total_cpu_time = 0;
    
    process_manager_initialized = true;
}

/*
 * Create a new process (summon a soul or demon)
 */
process_t* create_process(const char* name, uint64_t entry_point, process_priority_t priority, bool is_demon) {
    if (!process_manager_initialized || process_count >= MAX_PROCESSES) {
        return NULL;
    }
    
    // Find free process slot
    process_t* process = NULL;
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (processes[i].pid == 0) {
            process = &processes[i];
            break;
        }
    }
    
    if (!process) {
        return NULL;
    }
    
    // Initialize process
    process->pid = next_pid++;
    strncpy(process->name, name, PROCESS_NAME_LENGTH - 1);
    process->name[PROCESS_NAME_LENGTH - 1] = '\0';
    process->state = PROCESS_STATE_READY;
    process->priority = priority;
    process->is_demon = is_demon;
    process->is_kernel_mode = is_demon;
    process->is_suspended = false;
    
    // Set up memory
    process->stack_base = (uintptr_t)malloc(STACK_SIZE);
    if (!process->stack_base) {
        process->pid = 0;
        return NULL;
    }
    process->stack_pointer = process->stack_base + STACK_SIZE;
    process->heap_start = 0;
    process->heap_size = 0;
    
    // Set up execution context
    process->entry_point = entry_point;
    process->instruction_pointer = entry_point;
    for (int i = 0; i < 16; i++) {
        process->registers[i] = 0;
    }
    
    // Set up process relationships
    process->parent_pid = current_process ? current_process->pid : 0;
    process->parent = current_process;
    process->children = NULL;
    process->next_sibling = NULL;
    
    // Initialize scheduling data
    process->cpu_time = 0;
    process->last_scheduled = 0;
    process->time_slice = (priority == PRIORITY_OVERLORD) ? 100 : 
                         (priority == PRIORITY_DEMON) ? 50 : 
                         (priority == PRIORITY_SOUL) ? 25 : 10;
    
    // Add to process list
    process->next = process_list;
    process->prev = NULL;
    if (process_list) {
        process_list->prev = process;
    }
    process_list = process;
    
    // Add to ready queue
    add_to_ready_queue(process);
    
    // Update statistics
    process_count++;
    process_stats.total_processes++;
    process_stats.active_processes++;
    if (is_demon) {
        process_stats.demon_processes++;
    } else {
        process_stats.soul_processes++;
    }
    
    process->creation_time = get_system_time();
    
    return process;
}

/*
 * Terminate a process (banish a soul or demon)
 */
void terminate_process(uint32_t pid) {
    process_t* process = find_process_by_pid(pid);
    if (!process || process->state == PROCESS_STATE_TERMINATED) {
        return;
    }
    
    // Cannot terminate kernel process
    if (process->pid == 0) {
        return;
    }
    
    // Mark as terminated
    process->state = PROCESS_STATE_TERMINATED;
    
    // Remove from ready queue
    remove_from_ready_queue(process);
    
    // Free memory
    if (process->stack_base) {
        free((void*)(uintptr_t)process->stack_base);
        process->stack_base = 0;
    }
    
    if (process->heap_start) {
        free((void*)(uintptr_t)process->heap_start);
        process->heap_start = 0;
    }
    
    // Update statistics
    process_stats.active_processes--;
    if (process->is_demon) {
        process_stats.demon_processes--;
    } else {
        process_stats.soul_processes--;
    }
    
    // If this is the current process, schedule next
    if (process == current_process) {
        schedule_next_process();
    }
    
    // Clean up process entry
    process->pid = 0;
    process_count--;
}

/*
 * Find process by PID
 */
process_t* find_process_by_pid(uint32_t pid) {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (processes[i].pid == pid) {
            return &processes[i];
        }
    }
    return NULL;
}

/*
 * Add process to ready queue
 */
void add_to_ready_queue(process_t* process) {
    if (!process || process->state != PROCESS_STATE_READY) {
        return;
    }
    
    // Insert based on priority (higher priority first)
    if (!ready_queue || process->priority < ready_queue->priority) {
        process->next = ready_queue;
        ready_queue = process;
    } else {
        process_t* current = ready_queue;
        while (current->next && current->next->priority <= process->priority) {
            current = current->next;
        }
        process->next = current->next;
        current->next = process;
    }
}

/*
 * Remove process from ready queue
 */
void remove_from_ready_queue(process_t* process) {
    if (!process || !ready_queue) {
        return;
    }
    
    if (ready_queue == process) {
        ready_queue = process->next;
    } else {
        process_t* current = ready_queue;
        while (current->next && current->next != process) {
            current = current->next;
        }
        if (current->next) {
            current->next = process->next;
        }
    }
    
    process->next = NULL;
}

/*
 * Schedule next process
 */
void schedule_next_process(void) {
    if (!ready_queue) {
        return;
    }
    
    process_t* next_process = ready_queue;
    ready_queue = ready_queue->next;
    next_process->next = NULL;
    
    // Context switch
    if (current_process && current_process != next_process) {
        // Save current process context
        save_process_context(current_process);
        
        // If current process is still ready, add back to queue
        if (current_process->state == PROCESS_STATE_RUNNING) {
            current_process->state = PROCESS_STATE_READY;
            add_to_ready_queue(current_process);
        }
    }
    
    // Switch to next process
    current_process = next_process;
    current_process->state = PROCESS_STATE_RUNNING;
    current_process->last_scheduled = get_system_time();
    
    // Load process context
    load_process_context(current_process);
    
    process_stats.context_switches++;
}

/*
 * Save process context
 */
void save_process_context(process_t* process) {
    if (!process) {
        return;
    }
    
    // Save registers (simplified - would use assembly in real implementation)
    // This is a placeholder for the actual context saving
    process->cpu_time += get_system_time() - process->last_scheduled;
    process_stats.total_cpu_time += get_system_time() - process->last_scheduled;
}

/*
 * Load process context
 */
void load_process_context(process_t* process) {
    if (!process) {
        return;
    }
    
    // Load registers (simplified - would use assembly in real implementation)
    // This is a placeholder for the actual context loading
}

/*
 * Get current process
 */
process_t* get_current_process(void) {
    return current_process;
}

/*
 * Get process list
 */
process_t* get_process_list(void) {
    return process_list;
}

/*
 * Get process statistics
 */
process_stats_t* get_process_stats(void) {
    return &process_stats;
}

/*
 * Yield CPU to next process
 */
void yield_process(void) {
    if (current_process) {
        current_process->state = PROCESS_STATE_READY;
        schedule_next_process();
    }
}

/*
 * Suspend a process
 */
void suspend_process(uint32_t pid) {
    process_t* process = find_process_by_pid(pid);
    if (process && process->state == PROCESS_STATE_RUNNING) {
        process->is_suspended = true;
        process->state = PROCESS_STATE_WAITING;
        remove_from_ready_queue(process);
        
        if (process == current_process) {
            schedule_next_process();
        }
    }
}

/*
 * Resume a suspended process
 */
void resume_process(uint32_t pid) {
    process_t* process = find_process_by_pid(pid);
    if (process && process->is_suspended) {
        process->is_suspended = false;
        process->state = PROCESS_STATE_READY;
        add_to_ready_queue(process);
    }
}

/*
 * Get system time (placeholder)
 */
uint64_t get_system_time(void) {
    // In a real implementation, this would get the actual system time
    // For now, return a simple counter
    static uint64_t time_counter = 0;
    return ++time_counter;
}

/*
 * Process scheduler (called from timer interrupt)
 */
void process_scheduler(void) {
    if (!process_manager_initialized || !current_process) {
        return;
    }
    
    // Check if current process has exceeded its time slice
    uint64_t current_time = get_system_time();
    if (current_time - current_process->last_scheduled >= current_process->time_slice) {
        // Time slice expired, schedule next process
        if (current_process->state == PROCESS_STATE_RUNNING) {
            current_process->state = PROCESS_STATE_READY;
            schedule_next_process();
        }
    }
}

/*
 * List all processes (for shell command)
 */
void list_processes(void) {
    process_t* process = process_list;
    
    while (process) {
        // This would normally print to console
        // For now, just iterate through the list
        process = process->next;
    }
}

/*
 * Get process count
 */
uint32_t get_process_count(void) {
    return process_count;
} 