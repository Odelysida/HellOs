/*
 * HellOS Proprietary Graphics Driver
 * 680x480 resolution, 32-color palette
 */

#include "../../kernel/kernel.h"
#include "../../kernel/graphics.h"
#include "../../kernel/memory.h"
#include <stdint.h>

// Forward declaration
int abs(int x);

// Graphics state
static graphics_state_t graphics_state;
static uint8_t* framebuffer;
static uint32_t framebuffer_size;

// 32-color palette (RGB values)
static uint32_t hell_palette[32] = {
    0x000000, // COLOR_VOID_BLACK
    0xFF0000, // COLOR_HELL_RED
    0xFF4500, // COLOR_FLAME_ORANGE
    0x8B0000, // COLOR_DARK_RED
    0x2F0000, // COLOR_DEEP_CRIMSON
    0xFFFF00, // COLOR_SULFUR_YELLOW
    0xFF6600, // COLOR_EMBER_GLOW
    0x404040, // COLOR_SHADOW_GRAY
    0xFFFFFF, // COLOR_BONE_WHITE
    0x800000, // COLOR_BLOOD_MAROON
    0xFF8C00, // COLOR_FIRE_BRIGHT
    0x808080, // COLOR_ASH_GRAY
    0x800080, // COLOR_DEMON_PURPLE
    0x00FF00, // COLOR_TOXIC_GREEN
    0x8B4513, // COLOR_RUST_BROWN
    0xFF1493, // COLOR_MAGMA_RED
    // Extended palette
    0x1C1C1C, // Dark gray
    0x4B0000, // Very dark red
    0x660000, // Dark maroon
    0x990000, // Medium dark red
    0xCC0000, // Medium red
    0xFF3333, // Light red
    0xFF6666, // Lighter red
    0xFF9999, // Very light red
    0x331100, // Dark orange
    0x662200, // Medium dark orange
    0x993300, // Medium orange
    0xCC4400, // Light orange
    0xFF5500, // Bright orange
    0xFF7700, // Very bright orange
    0xFF9900, // Yellow orange
    0xFFBB00  // Light yellow orange
};

// Font data (8x8 bitmap font)
static uint8_t hell_font[256][8] = {
    // Basic ASCII characters - simplified for space
    // Character 'A' (0x41)
    [0x41] = {0x3C, 0x66, 0x66, 0x7E, 0x66, 0x66, 0x66, 0x00},
    // Character 'B' (0x42)
    [0x42] = {0x7C, 0x66, 0x66, 0x7C, 0x66, 0x66, 0x7C, 0x00},
    // Character 'C' (0x43)
    [0x43] = {0x3C, 0x66, 0x60, 0x60, 0x60, 0x66, 0x3C, 0x00},
    // Add more characters as needed...
    // For now, using a simple pattern for all characters
};

/*
 * Initialize the graphics system
 */
int init_hell_graphics_driver(void) {
    // Initialize graphics state
    graphics_state.width = SCREEN_WIDTH;
    graphics_state.height = SCREEN_HEIGHT;
    graphics_state.bpp = 8; // 8 bits per pixel for 256 colors (we use 32)
    graphics_state.initialized = false;
    
    // Calculate framebuffer size
    framebuffer_size = SCREEN_WIDTH * SCREEN_HEIGHT;
    
    // Allocate framebuffer memory
    framebuffer = (uint8_t*)0xB8000; // VGA memory for now
    
    // Set up VGA registers for our custom mode
    if (setup_vga_mode() != 0) {
        return HELL_ERROR_GRAPHICS;
    }
    
    // Initialize palette
    setup_hell_palette();
    
    // Clear screen
    clear_screen(COLOR_VOID_BLACK);
    
    graphics_state.initialized = true;
    return HELL_SUCCESS;
}

/*
 * Setup VGA mode for 680x480 (custom mode)
 */
int setup_vga_mode(void) {
    // VGA register programming for custom mode
    // This is a simplified version - real implementation would be more complex
    
    // Unlock VGA registers
    outb(0x3C2, 0x63); // Miscellaneous register
    
    // Sequencer registers
    outb(0x3C4, 0x00); outb(0x3C5, 0x03); // Reset
    outb(0x3C4, 0x01); outb(0x3C5, 0x01); // Clocking mode
    outb(0x3C4, 0x02); outb(0x3C5, 0x0F); // Plane mask
    outb(0x3C4, 0x03); outb(0x3C5, 0x00); // Character map select
    outb(0x3C4, 0x04); outb(0x3C5, 0x0E); // Memory mode
    
    // CRTC registers (simplified)
    outb(0x3D4, 0x11); outb(0x3D5, 0x00); // Unlock CRTC
    
    // Set horizontal timing for 680 pixels
    outb(0x3D4, 0x00); outb(0x3D5, 0x5F); // Horizontal total
    outb(0x3D4, 0x01); outb(0x3D5, 0x4F); // Horizontal display enable end
    outb(0x3D4, 0x02); outb(0x3D5, 0x50); // Horizontal blank start
    outb(0x3D4, 0x03); outb(0x3D5, 0x82); // Horizontal blank end
    
    // Set vertical timing for 480 lines
    outb(0x3D4, 0x06); outb(0x3D5, 0x0D); // Vertical total
    outb(0x3D4, 0x07); outb(0x3D5, 0x3E); // Overflow
    outb(0x3D4, 0x09); outb(0x3D5, 0x40); // Maximum scan line
    outb(0x3D4, 0x10); outb(0x3D5, 0xEA); // Vertical sync start
    outb(0x3D4, 0x11); outb(0x3D5, 0xAC); // Vertical sync end
    outb(0x3D4, 0x12); outb(0x3D5, 0xDF); // Vertical display enable end
    outb(0x3D4, 0x15); outb(0x3D5, 0xE7); // Vertical blank start
    outb(0x3D4, 0x16); outb(0x3D5, 0x06); // Vertical blank end
    
    // Graphics controller registers
    outb(0x3CE, 0x05); outb(0x3CF, 0x40); // Graphics mode
    outb(0x3CE, 0x06); outb(0x3CF, 0x05); // Miscellaneous
    
    // Attribute controller registers
    inb(0x3DA); // Reset flip-flop
    outb(0x3C0, 0x10); outb(0x3C0, 0x41); // Mode control
    outb(0x3C0, 0x11); outb(0x3C0, 0x00); // Overscan color
    outb(0x3C0, 0x12); outb(0x3C0, 0x0F); // Color plane enable
    outb(0x3C0, 0x13); outb(0x3C0, 0x00); // Horizontal pixel panning
    outb(0x3C0, 0x14); outb(0x3C0, 0x00); // Color select
    
    // Enable video
    inb(0x3DA);
    outb(0x3C0, 0x20);
    
    return HELL_SUCCESS;
}

/*
 * Setup the hellish color palette
 */
void setup_hell_palette(void) {
    // Program VGA palette registers
    for (int i = 0; i < 32; i++) {
        uint32_t color = hell_palette[i];
        uint8_t r = (color >> 16) & 0xFF;
        uint8_t g = (color >> 8) & 0xFF;
        uint8_t b = color & 0xFF;
        
        // Convert to 6-bit values for VGA
        r >>= 2;
        g >>= 2;
        b >>= 2;
        
        // Write to palette
        outb(0x3C8, i); // Palette index
        outb(0x3C9, r); // Red
        outb(0x3C9, g); // Green
        outb(0x3C9, b); // Blue
    }
}

/*
 * Clear the screen with a specific color
 */
void clear_screen(uint8_t color) {
    if (!graphics_state.initialized) return;
    
    // Fill framebuffer with color
    for (uint32_t i = 0; i < framebuffer_size; i++) {
        framebuffer[i] = color;
    }
}

/*
 * Draw a single pixel
 */
void draw_pixel(int x, int y, uint8_t color) {
    if (!graphics_state.initialized) return;
    if (x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGHT) return;
    
    int offset = y * SCREEN_WIDTH + x;
    framebuffer[offset] = color;
}

/*
 * Draw a line using Bresenham's algorithm
 */
void draw_line(int x0, int y0, int x1, int y1, uint8_t color) {
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;
    
    while (true) {
        draw_pixel(x0, y0, color);
        
        if (x0 == x1 && y0 == y1) break;
        
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}

/*
 * Draw a rectangle
 */
void draw_rectangle(int x, int y, int width, int height, uint8_t color) {
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            draw_pixel(x + j, y + i, color);
        }
    }
}

/*
 * Draw text using bitmap font
 */
void draw_text(const char* text, int x, int y, uint8_t color) {
    if (!graphics_state.initialized) return;
    
    int start_x = x;
    
    while (*text) {
        char c = *text;
        
        // Handle special characters
        if (c == '\n') {
            y += 8;
            x = start_x;
            text++;
            continue;
        }
        
        // Draw character
        draw_char(c, x, y, color);
        
        x += 8; // Move to next character position
        text++;
    }
}

/*
 * Draw a single character
 */
void draw_char(char c, int x, int y, uint8_t color) {
    uint8_t* glyph = hell_font[(unsigned char)c];
    
    for (int row = 0; row < 8; row++) {
        uint8_t line = glyph[row];
        for (int col = 0; col < 8; col++) {
            if (line & (0x80 >> col)) {
                draw_pixel(x + col, y + row, color);
            }
        }
    }
}

/*
 * Draw flame effect (animated)
 */
void draw_flame_effect(int x, int y, int width, int height) {
    static uint32_t flame_time = 0;
    flame_time++;
    
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            // Simple flame algorithm
            int intensity = (flame_time + i + j) % 32;
            uint8_t color;
            
            if (intensity < 8) {
                color = COLOR_HELL_RED;
            } else if (intensity < 16) {
                color = COLOR_FLAME_ORANGE;
            } else if (intensity < 24) {
                color = COLOR_EMBER_GLOW;
            } else {
                color = COLOR_SULFUR_YELLOW;
            }
            
            draw_pixel(x + j, y + i, color);
        }
    }
}

/*
 * Initialize graphics system
 */
void init_graphics_system(void) {
    // This is called from kernel initialization
    // Actual driver initialization happens in init_hell_graphics_driver
}

/*
 * Shutdown graphics driver
 */
void shutdown_graphics_driver(void) {
    if (graphics_state.initialized) {
        clear_screen(COLOR_VOID_BLACK);
        graphics_state.initialized = false;
    }
}

/*
 * Get current graphics state
 */
graphics_state_t* get_graphics_state(void) {
    return &graphics_state;
}

/*
 * Simple abs function
 */
int abs(int x) {
    return (x < 0) ? -x : x;
} 