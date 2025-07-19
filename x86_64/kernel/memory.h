/*
 * HellOS Memory Management Header
 * Memory subsystem definitions and structures
 */

#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>
#include <stdbool.h>

// Forward declarations
typedef struct memory_block_s memory_block_t;
typedef struct memory_stats_s memory_stats_t;

// Memory management functions
void init_memory_manager(void);
void* malloc(size_t size);
void free(void* ptr);
void* calloc(size_t num, size_t size);
void* realloc(void* ptr, size_t size);

// Memory utility functions
void* memset(void* ptr, int value, size_t size);
void* memcpy(void* dest, const void* src, size_t size);
int memcmp(const void* ptr1, const void* ptr2, size_t size);

// String functions
size_t strlen(const char* str);
int strcmp(const char* str1, const char* str2);
int strncmp(const char* str1, const char* str2, size_t n);
char* strcpy(char* dest, const char* src);
char* strncpy(char* dest, const char* src, size_t n);
char* strcat(char* dest, const char* src);

// Memory management internals
memory_block_t* find_free_block(size_t size);
void split_block(memory_block_t* block, size_t size);
void coalesce_blocks(memory_block_t* block);
bool validate_block(memory_block_t* block);
memory_stats_t* get_memory_stats(void);
void display_memory_info(void);

#endif // MEMORY_H 