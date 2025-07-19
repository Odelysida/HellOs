/*
 * HellOS Infernal Shell (InfSh)
 * The command interface to the underworld
 */

#include "../kernel/kernel.h"
#include "../kernel/memory.h"
#include "shell.h"
#include <stdint.h>

// Shell state
static shell_state_t shell_state;
static char command_buffer[256];
static int command_pos = 0;
static window_t* shell_window;

// Command history
#define MAX_HISTORY 16
static char command_history[MAX_HISTORY][256];
static int history_count = 0;
static int history_pos = 0;

// Built-in commands
typedef struct {
    const char* name;
    const char* description;
    void (*handler)(int argc, char** argv);
} shell_command_t;

// Forward declarations
void cmd_summon(int argc, char** argv);
void cmd_banish(int argc, char** argv);
void cmd_scry(int argc, char** argv);
void cmd_conjure(int argc, char** argv);
void cmd_purge(int argc, char** argv);
void cmd_realm(int argc, char** argv);
void cmd_souls(int argc, char** argv);
void cmd_demons(int argc, char** argv);
void cmd_inferno(int argc, char** argv);
void cmd_help(int argc, char** argv);
void cmd_about(int argc, char** argv);

// Built-in commands table
static shell_command_t builtin_commands[] = {
    {"summon", "Execute a soul (program)", cmd_summon},
    {"banish", "Terminate a demon (process)", cmd_banish},
    {"scry", "List contents of a realm (directory)", cmd_scry},
    {"conjure", "Create a new artifact (file)", cmd_conjure},
    {"purge", "Delete an artifact (file)", cmd_purge},
    {"realm", "Change current realm (directory)", cmd_realm},
    {"souls", "List active souls (processes)", cmd_souls},
    {"demons", "List system demons (system processes)", cmd_demons},
    {"inferno", "System information", cmd_inferno},
    {"help", "Show available incantations", cmd_help},
    {"about", "About HellOS", cmd_about},
    {NULL, NULL, NULL}
};

/*
 * Initialize the infernal shell
 */
void init_infernal_shell(window_t* window) {
    shell_window = window;
    
    // Initialize shell state
    shell_state.current_realm = "/abyss";
    shell_state.prompt_color = COLOR_FLAME_ORANGE;
    shell_state.text_color = COLOR_BONE_WHITE;
    shell_state.error_color = COLOR_HELL_RED;
    shell_state.initialized = true;
    
    // Clear command buffer
    memset(command_buffer, 0, sizeof(command_buffer));
    command_pos = 0;
    
    // Clear history
    memset(command_history, 0, sizeof(command_history));
    history_count = 0;
    history_pos = 0;
    
    // Display welcome message
    display_welcome_message();
    
    // Show initial prompt
    display_prompt();
}

/*
 * Display welcome message
 */
void display_welcome_message(void) {
    shell_print("🔥 Welcome to the Infernal Shell 🔥\n", COLOR_FLAME_ORANGE);
    shell_print("The gateway to the digital underworld\n", COLOR_HELL_RED);
    shell_print("Type 'help' for available incantations\n\n", COLOR_BONE_WHITE);
}

/*
 * Display shell prompt
 */
void display_prompt(void) {
    char prompt[128];
    snprintf(prompt, sizeof(prompt), "🔥 [%s] λ ", shell_state.current_realm);
    shell_print(prompt, shell_state.prompt_color);
}

/*
 * Process shell input
 */
void process_shell_input(char c) {
    if (!shell_state.initialized) return;
    
    switch (c) {
        case '\n':
        case '\r':
            // Execute command
            command_buffer[command_pos] = '\0';
            shell_print("\n", COLOR_BONE_WHITE);
            
            if (command_pos > 0) {
                add_to_history(command_buffer);
                execute_command(command_buffer);
            }
            
            // Reset command buffer
            command_pos = 0;
            memset(command_buffer, 0, sizeof(command_buffer));
            
            // Show new prompt
            display_prompt();
            break;
            
        case '\b':
        case 127: // DEL
            // Backspace
            if (command_pos > 0) {
                command_pos--;
                command_buffer[command_pos] = '\0';
                shell_print("\b \b", COLOR_BONE_WHITE); // Erase character
            }
            break;
            
        case '\t':
            // Tab completion (simplified)
            attempt_tab_completion();
            break;
            
        default:
            // Regular character
            if (command_pos < (int)sizeof(command_buffer) - 1 && c >= 32 && c <= 126) {
                command_buffer[command_pos] = c;
                command_pos++;
                
                // Echo character
                char echo[2] = {c, '\0'};
                shell_print(echo, COLOR_BONE_WHITE);
            }
            break;
    }
}

/*
 * Execute a command
 */
void execute_command(const char* command) {
    // Parse command into arguments
    char* args[16];
    int argc = 0;
    char cmd_copy[256];
    
    strncpy(cmd_copy, command, sizeof(cmd_copy));
    
    // Simple tokenization
    char* token = strtok(cmd_copy, " \t");
    while (token && argc < 15) {
        args[argc] = token;
        argc++;
        token = strtok(NULL, " \t");
    }
    args[argc] = NULL;
    
    if (argc == 0) return;
    
    // Find and execute command
    for (int i = 0; builtin_commands[i].name; i++) {
        if (strcmp(args[0], builtin_commands[i].name) == 0) {
            builtin_commands[i].handler(argc, args);
            return;
        }
    }
    
    // Command not found
    shell_print("Unknown incantation: ", shell_state.error_color);
    shell_print(args[0], shell_state.error_color);
    shell_print("\nType 'help' for available commands\n", shell_state.text_color);
}

/*
 * Add command to history
 */
void add_to_history(const char* command) {
    if (history_count < MAX_HISTORY) {
        strncpy(command_history[history_count], command, sizeof(command_history[0]));
        history_count++;
    } else {
        // Shift history
        for (int i = 0; i < MAX_HISTORY - 1; i++) {
            strncpy(command_history[i], command_history[i + 1], sizeof(command_history[0]));
        }
        strncpy(command_history[MAX_HISTORY - 1], command, sizeof(command_history[0]));
    }
    history_pos = history_count;
}

/*
 * Attempt tab completion
 */
void attempt_tab_completion(void) {
    // Simple completion for built-in commands
    for (int i = 0; builtin_commands[i].name; i++) {
        if (strncmp(command_buffer, builtin_commands[i].name, command_pos) == 0) {
            // Complete the command
            const char* completion = builtin_commands[i].name + command_pos;
            shell_print(completion, COLOR_BONE_WHITE);
            
            strcat(command_buffer, completion);
            command_pos = strlen(command_buffer);
            break;
        }
    }
}

/*
 * Print text to shell window
 */
void shell_print(const char* text, uint8_t color) {
    // For now, just draw to the graphics buffer
    // In a real implementation, this would be more sophisticated
    static int cursor_x = 10;
    static int cursor_y = 30;
    
    while (*text) {
        if (*text == '\n') {
            cursor_y += 12;
            cursor_x = 10;
        } else if (*text == '\b') {
            if (cursor_x > 10) {
                cursor_x -= 8;
                draw_char(' ', cursor_x, cursor_y, COLOR_VOID_BLACK);
            }
        } else {
            draw_char(*text, cursor_x, cursor_y, color);
            cursor_x += 8;
            
            if (cursor_x > shell_window->width - 20) {
                cursor_x = 10;
                cursor_y += 12;
            }
        }
        
        // Scroll if needed
        if (cursor_y > shell_window->height - 20) {
            cursor_y = 30;
            // Clear window (simplified)
            draw_rectangle(shell_window->x, shell_window->y, 
                          shell_window->width, shell_window->height, 
                          COLOR_VOID_BLACK);
        }
        
        text++;
    }
}

// Command implementations

void cmd_summon(int argc, char** argv) {
    if (argc < 2) {
        shell_print("Usage: summon <soul_name>\n", shell_state.error_color);
        return;
    }
    
    shell_print("Summoning soul: ", shell_state.text_color);
    shell_print(argv[1], COLOR_FLAME_ORANGE);
    shell_print("\n", shell_state.text_color);
    
    // TODO: Implement process execution
    shell_print("Soul summoning not yet implemented\n", shell_state.error_color);
}

void cmd_banish(int argc, char** argv) {
    if (argc < 2) {
        shell_print("Usage: banish <demon_id>\n", shell_state.error_color);
        return;
    }
    
    shell_print("Banishing demon: ", shell_state.text_color);
    shell_print(argv[1], COLOR_HELL_RED);
    shell_print("\n", shell_state.text_color);
    
    // TODO: Implement process termination
    shell_print("Demon banishment not yet implemented\n", shell_state.error_color);
}

void cmd_scry(int argc, char** argv) {
    const char* realm = (argc > 1) ? argv[1] : shell_state.current_realm;
    
    shell_print("Scrying realm: ", shell_state.text_color);
    shell_print(realm, COLOR_FLAME_ORANGE);
    shell_print("\n", shell_state.text_color);
    
    // TODO: Implement directory listing
    shell_print("Realm scrying not yet implemented\n", shell_state.error_color);
}

void cmd_conjure(int argc, char** argv) {
    if (argc < 2) {
        shell_print("Usage: conjure <artifact_name>\n", shell_state.error_color);
        return;
    }
    
    shell_print("Conjuring artifact: ", shell_state.text_color);
    shell_print(argv[1], COLOR_FLAME_ORANGE);
    shell_print("\n", shell_state.text_color);
    
    // TODO: Implement file creation
    shell_print("Artifact conjuring not yet implemented\n", shell_state.error_color);
}

void cmd_purge(int argc, char** argv) {
    if (argc < 2) {
        shell_print("Usage: purge <artifact_name>\n", shell_state.error_color);
        return;
    }
    
    shell_print("Purging artifact: ", shell_state.text_color);
    shell_print(argv[1], COLOR_HELL_RED);
    shell_print("\n", shell_state.text_color);
    
    // TODO: Implement file deletion
    shell_print("Artifact purging not yet implemented\n", shell_state.error_color);
}

void cmd_realm(int argc, char** argv) {
    (void)argv; // Suppress unused parameter warning
    if (argc < 2) {
        shell_print("Current realm: ", shell_state.text_color);
        shell_print(shell_state.current_realm, COLOR_FLAME_ORANGE);
        shell_print("\n", shell_state.text_color);
        return;
    }
    
    // TODO: Implement directory change
    shell_print("Realm traversal not yet implemented\n", shell_state.error_color);
}

void cmd_souls(int argc, char** argv) {
    (void)argc; // Suppress unused parameter warning
    (void)argv; // Suppress unused parameter warning
    shell_print("Active souls:\n", shell_state.text_color);
    shell_print("PID  Name           Status\n", COLOR_FLAME_ORANGE);
    shell_print("---  ----           ------\n", COLOR_FLAME_ORANGE);
    
    // TODO: Implement process listing
    shell_print("1    infernal_shell RUNNING\n", shell_state.text_color);
}

void cmd_demons(int argc, char** argv) {
    (void)argc; // Suppress unused parameter warning
    (void)argv; // Suppress unused parameter warning
    shell_print("System demons:\n", shell_state.text_color);
    shell_print("PID  Name           Status\n", COLOR_HELL_RED);
    shell_print("---  ----           ------\n", COLOR_HELL_RED);
    
    // TODO: Implement system process listing
    shell_print("0    kernel_daemon  RUNNING\n", shell_state.text_color);
}

void cmd_inferno(int argc, char** argv) {
    (void)argc; // Suppress unused parameter warning
    (void)argv; // Suppress unused parameter warning
    shell_print("=== INFERNO SYSTEM STATUS ===\n", COLOR_FLAME_ORANGE);
    shell_print("OS: HellOS - The Infernal Operating System\n", shell_state.text_color);
    shell_print("Kernel: Hell Kernel v0.1\n", shell_state.text_color);
    shell_print("Memory: Unknown\n", shell_state.text_color);
    shell_print("Graphics: 680x480, 32 colors\n", shell_state.text_color);
    shell_print("Audio: 3-channel (Square/Sine/Saw)\n", shell_state.text_color);
    shell_print("Network: Not initialized\n", shell_state.text_color);
    shell_print("Status: Burning bright 🔥\n", COLOR_HELL_RED);
}

void cmd_help(int argc, char** argv) {
    (void)argc; // Suppress unused parameter warning
    (void)argv; // Suppress unused parameter warning
    shell_print("=== INFERNAL INCANTATIONS ===\n", COLOR_FLAME_ORANGE);
    
    for (int i = 0; builtin_commands[i].name; i++) {
        shell_print(builtin_commands[i].name, COLOR_FLAME_ORANGE);
        shell_print(" - ", shell_state.text_color);
        shell_print(builtin_commands[i].description, shell_state.text_color);
        shell_print("\n", shell_state.text_color);
    }
    
    shell_print("\nMay your commands burn eternal! 🔥\n", COLOR_HELL_RED);
}

void cmd_about(int argc, char** argv) {
    (void)argc; // Suppress unused parameter warning
    (void)argv; // Suppress unused parameter warning
    shell_print("=== ABOUT HELLOS ===\n", COLOR_FLAME_ORANGE);
    shell_print("HellOS - The Infernal Operating System\n", COLOR_HELL_RED);
    shell_print("A hellish-themed OS inspired by TempleOS\n", shell_state.text_color);
    shell_print("Features:\n", shell_state.text_color);
    shell_print("• 680x480 graphics with 32-color palette\n", shell_state.text_color);
    shell_print("• 3-channel audio (Square/Sine/Sawtooth)\n", shell_state.text_color);
    shell_print("• Multi-window Pandemonium WM\n", shell_state.text_color);
    shell_print("• Network stack (TCP/UDP/Sockets)\n", shell_state.text_color);
    shell_print("• Infernal Shell with demonic commands\n", shell_state.text_color);
    shell_print("\nFrom the depths of silicon and fire! 🔥\n", COLOR_HELL_RED);
}

/*
 * Start shell process
 */
void start_shell_process(window_t* window) {
    init_infernal_shell(window);
    
    // Main shell loop would go here
    // For now, just initialize
}

/*
 * Simple string functions
 */
char* strtok(char* str, const char* delim) {
    static char* last = NULL;
    if (str) last = str;
    if (!last) return NULL;
    
    // Skip leading delimiters
    while (*last && strchr(delim, *last)) last++;
    if (!*last) return NULL;
    
    char* start = last;
    
    // Find next delimiter
    while (*last && !strchr(delim, *last)) last++;
    if (*last) {
        *last = '\0';
        last++;
    }
    
    return start;
}

char* strchr(const char* str, int c) {
    while (*str) {
        if (*str == c) return (char*)str;
        str++;
    }
    return (c == '\0') ? (char*)str : NULL;
}

int snprintf(char* str, size_t size, const char* format, ...) {
    // Simplified implementation
    // In a real OS, this would be much more complete
    strncpy(str, format, size);
    return strlen(str);
} 