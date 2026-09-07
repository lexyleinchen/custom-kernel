#ifndef UI_H
#define UI_H

#include <stdint.h>

struct Window;

typedef void (*WindowDrawCallback)(Window* window);

struct Window {
    int x;
    int y;
    int width;
    int height;
    const char* title;
    bool dragging;
    int drag_offset_x;
    int drag_offset_y;
    uint32_t background_color;
    uint32_t title_bar_color;
    uint32_t title_color;
    WindowDrawCallback draw_content;
};

bool graphics_button(int x, int y, int width, int height, uint32_t color, const char* text, uint32_t text_color);

Window ui_create_window(int x, int y, int width, int height, const char* title,uint32_t background_color, uint32_t title_bar_color, uint32_t title_color, WindowDrawCallback draw_content);

void ui_register_window(Window* window);

void ui_update_windows();

void ui_draw_windows();

int ui_window_content_x(const Window* window);

int ui_window_content_y(const Window* window);

int ui_window_content_width(const Window* window);

int ui_window_content_height(const Window* window);

#endif // UI_H