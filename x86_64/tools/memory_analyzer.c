/*
 * HellOS Memory Analyzer
 * Tool for analyzing memory dumps from HellOS
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>

#define MAX_DUMP_SIZE (1024 * 1024)  // 1MB max dump size

typedef struct {
    uint32_t address;
    uint8_t* data;
    size_t size;
} memory_dump_t;

// Display memory in hex format
void display_hex_dump(const memory_dump_t* dump, uint32_t start_offset, size_t length) {
    printf("Memory dump at 0x%08X (size: %zu bytes)\n", dump->address + start_offset, length);
    printf("Address    | 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F | ASCII\n");
    printf("-----------|--------------------------------------------------|------------------\n");
    
    for (size_t i = start_offset; i < start_offset + length && i < dump->size; i += 16) {
        printf("0x%08X | ", (uint32_t)(dump->address + i));
        
        // Hex bytes
        for (int j = 0; j < 16; j++) {
            if (i + j < dump->size) {
                printf("%02X ", dump->data[i + j]);
            } else {
                printf("   ");
            }
        }
        
        printf("| ");
        
        // ASCII representation
        for (int j = 0; j < 16; j++) {
            if (i + j < dump->size) {
                uint8_t byte = dump->data[i + j];
                printf("%c", (byte >= 32 && byte <= 126) ? byte : '.');
            } else {
                printf(" ");
            }
        }
        
        printf("\n");
    }
}

// Search for pattern in memory
void search_pattern(const memory_dump_t* dump, const char* pattern) {
    size_t pattern_len = strlen(pattern);
    int found = 0;
    
    printf("Searching for pattern: \"%s\"\n", pattern);
    
    for (size_t i = 0; i <= dump->size - pattern_len; i++) {
        if (memcmp(&dump->data[i], pattern, pattern_len) == 0) {
            printf("Found at 0x%08X (offset 0x%08X)\n", 
                   (uint32_t)(dump->address + i), (uint32_t)i);
            found = 1;
        }
    }
    
    if (!found) {
        printf("Pattern not found\n");
    }
}

// Analyze memory for common patterns
void analyze_memory(const memory_dump_t* dump) {
    printf("\n=== Memory Analysis ===\n");
    
    // Count zero bytes
    size_t zero_count = 0;
    for (size_t i = 0; i < dump->size; i++) {
        if (dump->data[i] == 0) {
            zero_count++;
        }
    }
    
    printf("Zero bytes: %zu (%.2f%%)\n", zero_count, 
           (double)zero_count / dump->size * 100.0);
    
    // Look for common patterns
    printf("\nCommon patterns:\n");
    
    // Stack patterns (0xDEADBEEF, 0xCAFEBABE)
    uint32_t deadbeef = 0xDEADBEEF;
    uint32_t cafebabe = 0xCAFEBABE;
    
    int deadbeef_count = 0;
    int cafebabe_count = 0;
    
    for (size_t i = 0; i <= dump->size - 4; i++) {
        uint32_t value = *(uint32_t*)&dump->data[i];
        if (value == deadbeef) deadbeef_count++;
        if (value == cafebabe) cafebabe_count++;
    }
    
    if (deadbeef_count > 0) {
        printf("  0xDEADBEEF: %d occurrences\n", deadbeef_count);
    }
    if (cafebabe_count > 0) {
        printf("  0xCAFEBABE: %d occurrences\n", cafebabe_count);
    }
    
    // Look for ASCII strings
    printf("\nASCII strings (min 4 chars):\n");
    char string_buffer[256];
    int string_pos = 0;
    
    for (size_t i = 0; i < dump->size; i++) {
        uint8_t byte = dump->data[i];
        
        if (byte >= 32 && byte <= 126) {
            // Printable ASCII
            if (string_pos < sizeof(string_buffer) - 1) {
                string_buffer[string_pos++] = byte;
            }
        } else {
            // Non-printable, end string
            if (string_pos >= 4) {
                string_buffer[string_pos] = '\0';
                printf("  0x%08X: \"%s\"\n", 
                       (uint32_t)(dump->address + i - string_pos), string_buffer);
            }
            string_pos = 0;
        }
    }
    
    // Final string if at end of dump
    if (string_pos >= 4) {
        string_buffer[string_pos] = '\0';
        printf("  0x%08X: \"%s\"\n", 
               (uint32_t)(dump->address + dump->size - string_pos), string_buffer);
    }
}

// Load memory dump from file
int load_memory_dump(const char* filename, memory_dump_t* dump, uint32_t base_address) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        perror("Failed to open dump file");
        return 0;
    }
    
    // Get file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    if (file_size > MAX_DUMP_SIZE) {
        fprintf(stderr, "Dump file too large: %ld bytes (max %d)\n", 
                file_size, MAX_DUMP_SIZE);
        fclose(file);
        return 0;
    }
    
    // Allocate memory
    dump->data = malloc(file_size);
    if (!dump->data) {
        perror("Failed to allocate memory for dump");
        fclose(file);
        return 0;
    }
    
    // Read file
    size_t bytes_read = fread(dump->data, 1, file_size, file);
    if (bytes_read != (size_t)file_size) {
        fprintf(stderr, "Failed to read complete dump file\n");
        free(dump->data);
        fclose(file);
        return 0;
    }
    
    dump->address = base_address;
    dump->size = file_size;
    
    fclose(file);
    printf("Loaded %zu bytes from %s (base address: 0x%08X)\n", 
           dump->size, filename, dump->address);
    return 1;
}

// Show help
void show_help(void) {
    printf("HellOS Memory Analyzer\n");
    printf("Usage: memory_analyzer [options] <dump_file>\n\n");
    printf("Options:\n");
    printf("  -a <address>   Base address for dump (default: 0x00000000)\n");
    printf("  -o <offset>    Start offset for display (default: 0)\n");
    printf("  -l <length>    Length to display (default: all)\n");
    printf("  -s <pattern>   Search for pattern in memory\n");
    printf("  -A             Perform automatic analysis\n");
    printf("  -h             Show this help\n\n");
    printf("Examples:\n");
    printf("  memory_analyzer memory.dump\n");
    printf("  memory_analyzer -a 0x8000 -o 0x100 -l 256 kernel.dump\n");
    printf("  memory_analyzer -s \"HellOS\" memory.dump\n");
    printf("  memory_analyzer -A memory.dump\n");
}

int main(int argc, char* argv[]) {
    uint32_t base_address = 0x00000000;
    uint32_t start_offset = 0;
    size_t display_length = 0;  // 0 means all
    char* search_pattern = NULL;
    int auto_analyze = 0;
    char* filename = NULL;
    
    // Parse command line arguments
    int opt;
    while ((opt = getopt(argc, argv, "a:o:l:s:Ah")) != -1) {
        switch (opt) {
            case 'a':
                base_address = strtoul(optarg, NULL, 0);
                break;
            case 'o':
                start_offset = strtoul(optarg, NULL, 0);
                break;
            case 'l':
                display_length = strtoul(optarg, NULL, 0);
                break;
            case 's':
                search_pattern = optarg;
                break;
            case 'A':
                auto_analyze = 1;
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
        fprintf(stderr, "Error: No dump file specified\n\n");
        show_help();
        return 1;
    }
    filename = argv[optind];
    
    // Load memory dump
    memory_dump_t dump;
    if (!load_memory_dump(filename, &dump, base_address)) {
        return 1;
    }
    
    // Validate offset and length
    if (start_offset >= dump.size) {
        fprintf(stderr, "Error: Start offset 0x%X is beyond dump size 0x%zX\n", 
                start_offset, dump.size);
        free(dump.data);
        return 1;
    }
    
    if (display_length == 0) {
        display_length = dump.size - start_offset;
    } else if (start_offset + display_length > dump.size) {
        display_length = dump.size - start_offset;
    }
    
    // Perform requested operations
    if (search_pattern) {
        search_pattern(&dump, search_pattern);
    } else if (auto_analyze) {
        analyze_memory(&dump);
    } else {
        display_hex_dump(&dump, start_offset, display_length);
    }
    
    free(dump.data);
    return 0;
} 