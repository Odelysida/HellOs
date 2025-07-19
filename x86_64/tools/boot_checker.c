/*
 * HellOS Boot Checker
 * Tool for validating boot images and bootloaders
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>

#define BOOT_SECTOR_SIZE 512
#define BOOT_SIGNATURE 0xAA55

typedef struct {
    uint8_t data[BOOT_SECTOR_SIZE];
    int valid;
} boot_sector_t;

// Check if boot sector has valid signature
int check_boot_signature(const boot_sector_t* sector) {
    uint16_t signature = *(uint16_t*)&sector->data[510];
    return signature == BOOT_SIGNATURE;
}

// Analyze boot sector
void analyze_boot_sector(const boot_sector_t* sector) {
    printf("=== Boot Sector Analysis ===\n");
    
    // Check signature
    if (check_boot_signature(sector)) {
        printf("✓ Boot signature: Valid (0xAA55)\n");
    } else {
        printf("✗ Boot signature: Invalid (0x%04X)\n", 
               *(uint16_t*)&sector->data[510]);
    }
    
    // Check for common bootloader patterns
    if (memcmp(&sector->data[0], "\xEB", 1) == 0 || 
        memcmp(&sector->data[0], "\xE9", 1) == 0) {
        printf("✓ Jump instruction: Found at start\n");
    } else {
        printf("? Jump instruction: Not found at start (0x%02X)\n", sector->data[0]);
    }
    
    // Look for HellOS strings
    if (memmem(sector->data, BOOT_SECTOR_SIZE, "HellOS", 6)) {
        printf("✓ HellOS identifier: Found\n");
    } else {
        printf("? HellOS identifier: Not found\n");
    }
    
    // Check for common x86 instructions
    int instruction_count = 0;
    for (int i = 0; i < BOOT_SECTOR_SIZE - 1; i++) {
        // Look for common x86 opcodes
        if (sector->data[i] == 0xFA) { // CLI
            instruction_count++;
        } else if (sector->data[i] == 0xFB) { // STI
            instruction_count++;
        } else if (sector->data[i] == 0xCD) { // INT
            instruction_count++;
        } else if (sector->data[i] == 0xE8) { // CALL
            instruction_count++;
        }
    }
    
    if (instruction_count > 5) {
        printf("✓ x86 instructions: Found %d common instructions\n", instruction_count);
    } else {
        printf("? x86 instructions: Only %d found (may not be x86 code)\n", instruction_count);
    }
    
    // Check for string data
    int string_count = 0;
    for (int i = 0; i < BOOT_SECTOR_SIZE - 4; i++) {
        if (sector->data[i] >= 32 && sector->data[i] <= 126) {
            // Check if we have at least 4 consecutive printable characters
            int consecutive = 0;
            for (int j = i; j < BOOT_SECTOR_SIZE && consecutive < 4; j++) {
                if (sector->data[j] >= 32 && sector->data[j] <= 126) {
                    consecutive++;
                } else {
                    break;
                }
            }
            if (consecutive >= 4) {
                string_count++;
                i += consecutive - 1; // Skip ahead
            }
        }
    }
    
    if (string_count > 0) {
        printf("✓ String data: Found %d potential strings\n", string_count);
    } else {
        printf("? String data: No strings found\n");
    }
    
    // Calculate code density (non-zero bytes)
    int non_zero_bytes = 0;
    for (int i = 0; i < BOOT_SECTOR_SIZE; i++) {
        if (sector->data[i] != 0) {
            non_zero_bytes++;
        }
    }
    
    double density = (double)non_zero_bytes / BOOT_SECTOR_SIZE * 100.0;
    printf("✓ Code density: %.1f%% (%d non-zero bytes)\n", density, non_zero_bytes);
    
    if (density < 10.0) {
        printf("⚠ Warning: Low code density, may not be a valid bootloader\n");
    }
}

// Display boot sector in hex
void display_boot_sector(const boot_sector_t* sector) {
    printf("\n=== Boot Sector Hex Dump ===\n");
    printf("Offset | 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F | ASCII\n");
    printf("-------|--------------------------------------------------|------------------\n");
    
    for (int i = 0; i < BOOT_SECTOR_SIZE; i += 16) {
        printf("0x%04X | ", i);
        
        // Hex bytes
        for (int j = 0; j < 16; j++) {
            printf("%02X ", sector->data[i + j]);
        }
        
        printf("| ");
        
        // ASCII representation
        for (int j = 0; j < 16; j++) {
            uint8_t byte = sector->data[i + j];
            printf("%c", (byte >= 32 && byte <= 126) ? byte : '.');
        }
        
        printf("\n");
    }
}

// Extract strings from boot sector
void extract_strings(const boot_sector_t* sector, int min_length) {
    printf("\n=== Extracted Strings (min %d chars) ===\n", min_length);
    
    char string_buffer[256];
    int string_pos = 0;
    int string_count = 0;
    
    for (int i = 0; i < BOOT_SECTOR_SIZE; i++) {
        uint8_t byte = sector->data[i];
        
        if (byte >= 32 && byte <= 126) {
            // Printable ASCII
            if (string_pos < sizeof(string_buffer) - 1) {
                string_buffer[string_pos++] = byte;
            }
        } else {
            // Non-printable, end string
            if (string_pos >= min_length) {
                string_buffer[string_pos] = '\0';
                printf("0x%04X: \"%s\"\n", i - string_pos, string_buffer);
                string_count++;
            }
            string_pos = 0;
        }
    }
    
    // Final string if at end of sector
    if (string_pos >= min_length) {
        string_buffer[string_pos] = '\0';
        printf("0x%04X: \"%s\"\n", BOOT_SECTOR_SIZE - string_pos, string_buffer);
        string_count++;
    }
    
    if (string_count == 0) {
        printf("No strings found with minimum length %d\n", min_length);
    }
}

// Load boot sector from file
int load_boot_sector(const char* filename, boot_sector_t* sector) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        perror("Failed to open boot image");
        return 0;
    }
    
    size_t bytes_read = fread(sector->data, 1, BOOT_SECTOR_SIZE, file);
    fclose(file);
    
    if (bytes_read != BOOT_SECTOR_SIZE) {
        if (bytes_read == 0) {
            fprintf(stderr, "Error: File is empty\n");
        } else {
            fprintf(stderr, "Warning: File is %zu bytes, expected %d bytes\n", 
                    bytes_read, BOOT_SECTOR_SIZE);
            // Pad with zeros
            memset(&sector->data[bytes_read], 0, BOOT_SECTOR_SIZE - bytes_read);
        }
    }
    
    sector->valid = check_boot_signature(sector);
    
    printf("Loaded %zu bytes from %s\n", bytes_read, filename);
    return 1;
}

// Show help
void show_help(void) {
    printf("HellOS Boot Checker\n");
    printf("Usage: boot_checker [options] <boot_image>\n\n");
    printf("Options:\n");
    printf("  -a             Analyze boot sector\n");
    printf("  -d             Display hex dump\n");
    printf("  -s [length]    Extract strings (default min length: 4)\n");
    printf("  -h             Show this help\n\n");
    printf("Examples:\n");
    printf("  boot_checker hellboot.bin\n");
    printf("  boot_checker -a -d hellboot.bin\n");
    printf("  boot_checker -s 6 hellboot.bin\n");
}

int main(int argc, char* argv[]) {
    int analyze = 0;
    int hex_dump = 0;
    int extract_str = 0;
    int min_str_length = 4;
    char* filename = NULL;
    
    // Parse command line arguments
    int opt;
    while ((opt = getopt(argc, argv, "ads:h")) != -1) {
        switch (opt) {
            case 'a':
                analyze = 1;
                break;
            case 'd':
                hex_dump = 1;
                break;
            case 's':
                extract_str = 1;
                if (optarg) {
                    min_str_length = atoi(optarg);
                    if (min_str_length < 1) {
                        min_str_length = 1;
                    }
                }
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
        fprintf(stderr, "Error: No boot image specified\n\n");
        show_help();
        return 1;
    }
    filename = argv[optind];
    
    // Load boot sector
    boot_sector_t sector;
    if (!load_boot_sector(filename, &sector)) {
        return 1;
    }
    
    // If no specific options, do basic analysis
    if (!analyze && !hex_dump && !extract_str) {
        analyze = 1;
    }
    
    // Perform requested operations
    if (analyze) {
        analyze_boot_sector(&sector);
    }
    
    if (hex_dump) {
        display_boot_sector(&sector);
    }
    
    if (extract_str) {
        extract_strings(&sector, min_str_length);
    }
    
    return 0;
} 