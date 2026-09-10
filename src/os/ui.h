#ifndef UI_H
#define UI_H

#include <stdint.h>

struct Window;

typedef void (*WindowUpdateCallback)(Window* window);
typedef void (*WindowDrawCallback)(Window* window);

struct Scrollbar {
    bool dragging;
    int drag_offset;
};

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
    WindowUpdateCallback update_content;
    WindowDrawCallback draw_content;
    Scrollbar scrollbar;
};

bool graphics_button(int x, int y, int width, int height, uint32_t color, const char* text, uint32_t text_color);

Window ui_create_window(int x, int y, int width, int height, const char* title,uint32_t background_color, uint32_t title_bar_color, uint32_t title_color, WindowUpdateCallback update_content, WindowDrawCallback draw_content);

void ui_register_window(Window* window);

void ui_update_windows();

void ui_draw_windows();

int ui_window_content_x(const Window* window);

int ui_window_content_y(const Window* window);

int ui_window_content_width(const Window* window);

int ui_window_content_height(const Window* window);

void draw_scrollbar(int x, int y, int width, int height, int total_lines, int visible_lines, int scroll);

void update_scrollbar(Window* window, int x, int y , int width, int height, int total_lines, int visible_lines, int& scroll);

void font_draw_progressbar(int x, int y, int width, int height, int filled, uint32_t background_color, uint32_t fill_color);

#endif // UI_H