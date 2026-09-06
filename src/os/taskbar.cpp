#include "taskbar.h"
#include "graphics.h"
#include "ui.h"
#include "font.h"

static bool start_menu_open = false;

void taskbar_init() {
    start_menu_open = false;
}

static void draw_start_menu() {
    int menu_width = 250;
    int button_height = 40;
    int spacing = 10;
    int screen_height = graphics_height();
    int menu_height = (button_height * 2) + (spacing * 2) + (15 * 2) + 10;
    int menu_x = 10;
    int menu_y = screen_height - TASKBAR_HEIGHT - menu_height;
    graphics_rectangle(menu_x, menu_y, menu_width, menu_height, 0xFF404040);
    font_draw_text(menu_x + (menu_width - ((7 * 10) + (6 * 4))) / 2, menu_y + 15, "PrintOS", 0xFFFFFFFF);

    if (graphics_button(menu_x + 15, menu_y + 15 + spacing + 10, menu_width - 30, button_height, 0xFF606060, "Restart", 0xFFFFFFFF)) {
        // restart
    }

    if (graphics_button(menu_x + 15, menu_y + 15 + spacing * 2 + button_height + 10, menu_width - 30, button_height, 0xFF606060, "Shutdown", 0xFFFFFFFF)) {
        // shutdown
    }
}

void taskbar_draw() {
    int screen_width = graphics_width();
    int screen_height = graphics_height();
    graphics_rectangle(0, screen_height - TASKBAR_HEIGHT, screen_width, TASKBAR_HEIGHT, 0xFF808080); // Draw the taskbar background (gray)
    int start_x = 10;
    int start_y = screen_height - TASKBAR_HEIGHT + 10;
    
    if (graphics_button(start_x, start_y, 100, 30, 0xFF606060, "Start", 0xFFFFFFFF)) {
        start_menu_open = !start_menu_open;
    }

    if (start_menu_open) {
        draw_start_menu();
    }
}