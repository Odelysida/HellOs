/*
 * HellOS Shell System Header
 * Shell subsystem definitions and structures
 */

#ifndef SHELL_H
#define SHELL_H

#include <stdint.h>
#include <stdbool.h>

// Shell state structure
typedef struct {
    const char* current_realm;
    uint8_t prompt_color;
    uint8_t text_color;
    uint8_t error_color;
    bool initialized;
} shell_state_t;

// Function prototypes
void init_infernal_shell(window_t* window);
void display_welcome_message(void);
void display_prompt(void);
void process_shell_input(char c);
void execute_command(const char* command);
void add_to_history(const char* command);
void attempt_tab_completion(void);
void shell_print(const char* text, uint8_t color);

// String functions
char* strtok(char* str, const char* delim);
char* strchr(const char* str, int c);
int snprintf(char* str, size_t size, const char* format, ...);

#endif // SHELL_H 