#include "graphics.h"
#include "taskbar.h"

static Graphics graphics;

// Maximal supported resolution by the back buffer.
#define BACKBUFFER_WIDTH 1920
#define BACKBUFFER_HEIGHT 1080

static uint32_t backbuffer[BACKBUFFER_WIDTH * BACKBUFFER_HEIGHT];

void graphics_init(Graphics new_graphics) {
    graphics = new_graphics;

    for (uint32_t y = 0 ; y < graphics.height; y++) {
        for (uint32_t x = 0; x < graphics.width; x++) {
            backbuffer[y * BACKBUFFER_WIDTH + x] = 0xFF000000;
        }
    }
}

void graphics_clear(uint32_t color) {
    if (graphics.framebuffer == nullptr) {
        return;
    }

    if (graphics.width > BACKBUFFER_WIDTH || graphics.height > BACKBUFFER_HEIGHT) {
        return;
    }

    for (uint32_t y = 0; y < graphics.height; y++) {
        for (uint32_t x = 0; x < graphics.width; x++) {
            backbuffer[y * BACKBUFFER_WIDTH + x] = color;
        }
    }
}

void graphics_rectangle(int x, int y, int width, int height, uint32_t color) {
    if (graphics.framebuffer == nullptr) {
        return;
    }

    for (int yy = 0; yy < height; yy++) {
        for (int xx = 0; xx < width; xx++) {
            int px = x + xx;
            int py = y + yy;

            if (px >= 0 && px < static_cast<int>(graphics.width) && py >= 0 && py < static_cast<int>(graphics.height)) {
                backbuffer[py * BACKBUFFER_WIDTH + px] = color;
            }
        }
    }
}

uint32_t graphics_width() {
    return graphics.width;
}

uint32_t graphics_height() {
    return graphics.height;
}

uint32_t desktop_height() {
    return graphics.height - TASKBAR_HEIGHT;
}

void graphics_present() {
    if (graphics.framebuffer == nullptr) {
        return;
    }

    if (graphics.width > BACKBUFFER_WIDTH || graphics.height > BACKBUFFER_HEIGHT) {
        return;
    }

    for (uint32_t y = 0; y < graphics.height; y++) {
        uint32_t* destination = graphics.framebuffer + (y * graphics.pitch / sizeof(uint32_t));
        uint32_t* source = backbuffer + (y * BACKBUFFER_WIDTH);

        for (uint32_t x = 0; x < graphics.width; x++) {
            destination[x] = source[x];
        }
    }
}