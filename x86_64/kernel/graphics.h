/*
 * HellOS Graphics System Header
 * Graphics subsystem definitions and structures
 */

#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stdint.h>
#include <stdbool.h>

// Graphics state structure
typedef struct {
    uint32_t width;
    uint32_t height;
    uint8_t bpp;
    bool initialized;
} graphics_state_t;

// Function prototypes
int setup_vga_mode(void);
void setup_hell_palette(void);
void draw_line(int x0, int y0, int x1, int y1, uint8_t color);
void draw_rectangle(int x, int y, int width, int height, uint8_t color);
void draw_char(char c, int x, int y, uint8_t color);
void draw_flame_effect(int x, int y, int width, int height);
void shutdown_graphics_driver(void);
graphics_state_t* get_graphics_state(void);

#endif // GRAPHICS_H 