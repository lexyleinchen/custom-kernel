#include "os.h"
#include "graphics.h"
#include "taskbar.h"
#include "os_mouse.h"
#include "app_registry.h"
#include "font.h"
#include "ui.h"

#include "../kernel/framebuffer.h"
#include "../kernel/log.h"
#include "../kernel/mouse.h"

extern "C"
void os_init(void) {
    kernel_log("initializing os...");
    // Initialize the framebuffer and graphics
    Framebuffer framebuffer = framebuffer_get();
    Graphics graphics;
    graphics.framebuffer = framebuffer.address;
    graphics.width = framebuffer.width;
    graphics.height = framebuffer.height;
    graphics.pitch = framebuffer.pitch;
    graphics_init(graphics);

    // OS initialization
    mouse_init(framebuffer.width, framebuffer.height);
    taskbar_init();
    
    for (int i = 0; i < app_count(); i++) {
        app_get(i)->init();
    }

    kernel_log("os started.");
}

extern "C"
void os_draw(void) {
    graphics_clear(0xFF377c82); // Clear the screen with a color (blue)
    ui_update_windows();
    ui_draw_windows();
    taskbar_draw();
    mouse_draw();
    graphics_present();
}