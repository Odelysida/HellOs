/*
 * HellOS Memory Management System
 * The infernal heap and paging system
 */

#include "kernel.h"
#include "memory.h"
#include <stdint.h>

// Memory constants
#define PAGE_SIZE 4096
#define HEAP_START 0x200000  // 2MB
#define HEAP_SIZE 0x800000   // 8MB heap
#define MAX_BLOCKS 1024

// Memory block structure for heap management
struct memory_block_s {
    uint32_t size;
    bool is_free;
    struct memory_block_s* next;
    struct memory_block_s* prev;
    uint32_t magic;  // Magic number for corruption detection
};

// Memory statistics
struct memory_stats_s {
    uint64_t total_memory;
    uint64_t used_memory;
    uint64_t free_memory;
    uint32_t allocated_blocks;
    uint32_t free_blocks;
    uint32_t corrupted_blocks;
};

// Global memory state
static memory_block_t* heap_start = NULL;
static memory_block_t* heap_end = NULL;
static memory_stats_t memory_stats = {0};
static bool memory_initialized = false;

// Magic numbers for corruption detection
#define BLOCK_MAGIC_ALLOCATED 0xDEADBEEF
#define BLOCK_MAGIC_FREE      0xFEEDFACE
#define HEAP_MAGIC            0xHELL0666

/*
 * Initialize the memory management system
 */
void init_memory_manager(void) {
    // Initialize heap
    heap_start = (memory_block_t*)HEAP_START;
    heap_end = (memory_block_t*)(HEAP_START + HEAP_SIZE);
    
    // Create initial free block
    heap_start->size = HEAP_SIZE - sizeof(memory_block_t);
    heap_start->is_free = true;
    heap_start->next = NULL;
    heap_start->prev = NULL;
    heap_start->magic = BLOCK_MAGIC_FREE;
    
    // Initialize statistics
    memory_stats.total_memory = HEAP_SIZE;
    memory_stats.used_memory = sizeof(memory_block_t);
    memory_stats.free_memory = HEAP_SIZE - sizeof(memory_block_t);
    memory_stats.allocated_blocks = 0;
    memory_stats.free_blocks = 1;
    memory_stats.corrupted_blocks = 0;
    
    memory_initialized = true;
}

/*
 * Allocate memory from the infernal heap
 */
void* malloc(size_t size) {
    if (!memory_initialized) {
        return NULL;
    }
    
    if (size == 0) {
        return NULL;
    }
    
    // Align size to 8-byte boundary
    size = ALIGN_UP(size, 8);
    
    // Find a suitable free block
    memory_block_t* block = find_free_block(size);
    if (!block) {
        return NULL;  // Out of memory
    }
    
    // Split block if necessary
    if (block->size > size + sizeof(memory_block_t) + 8) {
        split_block(block, size);
    }
    
    // Mark block as allocated
    block->is_free = false;
    block->magic = BLOCK_MAGIC_ALLOCATED;
    
    // Update statistics
    memory_stats.used_memory += block->size;
    memory_stats.free_memory -= block->size;
    memory_stats.allocated_blocks++;
    memory_stats.free_blocks--;
    
    // Return pointer to data (after header)
    return (void*)((uint8_t*)block + sizeof(memory_block_t));
}

/*
 * Free memory back to the infernal heap
 */
void free(void* ptr) {
    if (!ptr || !memory_initialized) {
        return;
    }
    
    // Get block header
    memory_block_t* block = (memory_block_t*)((uint8_t*)ptr - sizeof(memory_block_t));
    
    // Validate block
    if (!validate_block(block)) {
        memory_stats.corrupted_blocks++;
        return;  // Corrupted block
    }
    
    // Mark as free
    block->is_free = true;
    block->magic = BLOCK_MAGIC_FREE;
    
    // Update statistics
    memory_stats.used_memory -= block->size;
    memory_stats.free_memory += block->size;
    memory_stats.allocated_blocks--;
    memory_stats.free_blocks++;
    
    // Coalesce with adjacent free blocks
    coalesce_blocks(block);
}

/*
 * Find a free block of sufficient size
 */
memory_block_t* find_free_block(size_t size) {
    memory_block_t* current = heap_start;
    
    while (current && current < heap_end) {
        if (current->is_free && current->size >= size) {
            return current;
        }
        current = current->next;
    }
    
    return NULL;  // No suitable block found
}

/*
 * Split a block into two parts
 */
void split_block(memory_block_t* block, size_t size) {
    if (!block || block->size <= size + sizeof(memory_block_t)) {
        return;
    }
    
    // Create new block
    memory_block_t* new_block = (memory_block_t*)((uint8_t*)block + sizeof(memory_block_t) + size);
    new_block->size = block->size - size - sizeof(memory_block_t);
    new_block->is_free = true;
    new_block->magic = BLOCK_MAGIC_FREE;
    new_block->next = block->next;
    new_block->prev = block;
    
    // Update original block
    block->size = size;
    block->next = new_block;
    
    // Update next block's prev pointer
    if (new_block->next) {
        new_block->next->prev = new_block;
    }
    
    memory_stats.free_blocks++;
}

/*
 * Coalesce adjacent free blocks
 */
void coalesce_blocks(memory_block_t* block) {
    if (!block || !block->is_free) {
        return;
    }
    
    // Coalesce with next block
    if (block->next && block->next->is_free) {
        block->size += block->next->size + sizeof(memory_block_t);
        memory_block_t* next = block->next;
        block->next = next->next;
        if (next->next) {
            next->next->prev = block;
        }
        memory_stats.free_blocks--;
    }
    
    // Coalesce with previous block
    if (block->prev && block->prev->is_free) {
        block->prev->size += block->size + sizeof(memory_block_t);
        block->prev->next = block->next;
        if (block->next) {
            block->next->prev = block->prev;
        }
        memory_stats.free_blocks--;
    }
}

/*
 * Validate a memory block
 */
bool validate_block(memory_block_t* block) {
    if (!block) {
        return false;
    }
    
    // Check if block is within heap bounds
    if (block < heap_start || block >= heap_end) {
        return false;
    }
    
    // Check magic number
    if (block->magic != BLOCK_MAGIC_ALLOCATED && block->magic != BLOCK_MAGIC_FREE) {
        return false;
    }
    
    return true;
}

/*
 * Get memory statistics
 */
memory_stats_t* get_memory_stats(void) {
    return &memory_stats;
}

/*
 * Memory set function
 */
void* memset(void* ptr, int value, size_t size) {
    uint8_t* p = (uint8_t*)ptr;
    uint8_t val = (uint8_t)value;
    
    for (size_t i = 0; i < size; i++) {
        p[i] = val;
    }
    
    return ptr;
}

/*
 * Memory copy function
 */
void* memcpy(void* dest, const void* src, size_t size) {
    uint8_t* d = (uint8_t*)dest;
    const uint8_t* s = (const uint8_t*)src;
    
    for (size_t i = 0; i < size; i++) {
        d[i] = s[i];
    }
    
    return dest;
}

/*
 * Memory compare function
 */
int memcmp(const void* ptr1, const void* ptr2, size_t size) {
    const uint8_t* p1 = (const uint8_t*)ptr1;
    const uint8_t* p2 = (const uint8_t*)ptr2;
    
    for (size_t i = 0; i < size; i++) {
        if (p1[i] != p2[i]) {
            return p1[i] - p2[i];
        }
    }
    
    return 0;
}

/*
 * String length function
 */
size_t strlen(const char* str) {
    size_t len = 0;
    while (str[len]) {
        len++;
    }
    return len;
}

/*
 * String compare function
 */
int strcmp(const char* str1, const char* str2) {
    while (*str1 && *str2 && *str1 == *str2) {
        str1++;
        str2++;
    }
    return *str1 - *str2;
}

/*
 * String compare function (limited)
 */
int strncmp(const char* str1, const char* str2, size_t n) {
    for (size_t i = 0; i < n; i++) {
        if (str1[i] != str2[i] || str1[i] == '\0') {
            return str1[i] - str2[i];
        }
    }
    return 0;
}

/*
 * String copy function
 */
char* strcpy(char* dest, const char* src) {
    char* d = dest;
    while ((*d++ = *src++));
    return dest;
}

/*
 * String copy function (limited)
 */
char* strncpy(char* dest, const char* src, size_t n) {
    size_t i;
    for (i = 0; i < n && src[i]; i++) {
        dest[i] = src[i];
    }
    for (; i < n; i++) {
        dest[i] = '\0';
    }
    return dest;
}

/*
 * String concatenate function
 */
char* strcat(char* dest, const char* src) {
    char* d = dest;
    while (*d) d++;
    while ((*d++ = *src++));
    return dest;
}

/*
 * Allocate zeroed memory
 */
void* calloc(size_t num, size_t size) {
    size_t total_size = num * size;
    void* ptr = malloc(total_size);
    
    if (ptr) {
        memset(ptr, 0, total_size);
    }
    
    return ptr;
}

/*
 * Reallocate memory
 */
void* realloc(void* ptr, size_t size) {
    if (!ptr) {
        return malloc(size);
    }
    
    if (size == 0) {
        free(ptr);
        return NULL;
    }
    
    // Get current block
    memory_block_t* block = (memory_block_t*)((uint8_t*)ptr - sizeof(memory_block_t));
    
    if (!validate_block(block)) {
        return NULL;
    }
    
    // If new size fits in current block, just return it
    if (size <= block->size) {
        return ptr;
    }
    
    // Allocate new block
    void* new_ptr = malloc(size);
    if (!new_ptr) {
        return NULL;
    }
    
    // Copy old data
    memcpy(new_ptr, ptr, block->size);
    
    // Free old block
    free(ptr);
    
    return new_ptr;
}

/*
 * Display memory usage (for debugging)
 */
void display_memory_info(void) {
    if (!memory_initialized) {
        return;
    }
    
    // This would normally print to console
    // For now, just update statistics
    memory_stats.total_memory = HEAP_SIZE;
    
    // Walk through blocks to get accurate stats
    memory_block_t* current = heap_start;
    uint32_t free_count = 0;
    uint32_t allocated_count = 0;
    uint64_t free_bytes = 0;
    uint64_t allocated_bytes = 0;
    
    while (current && current < heap_end) {
        if (validate_block(current)) {
            if (current->is_free) {
                free_count++;
                free_bytes += current->size;
            } else {
                allocated_count++;
                allocated_bytes += current->size;
            }
        }
        current = current->next;
    }
    
    memory_stats.free_blocks = free_count;
    memory_stats.allocated_blocks = allocated_count;
    memory_stats.free_memory = free_bytes;
    memory_stats.used_memory = allocated_bytes;
} 