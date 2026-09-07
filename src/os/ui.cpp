#include "ui.h"
#include "graphics.h"
#include "font.h"

#include "../kernel/mouse.h"

#define WINDOW_TITLE_BAR_HEIGHT 26
#define WINDOW_BORDER_SIZE 1
#define MAX_WINDOWS 32

static Window* windows[MAX_WINDOWS];
static int window_count;
static Window* focused_window = nullptr;

bool graphics_button(int x, int y, int width, int height, uint32_t color, const char* text, uint32_t text_color) {
    MouseState mouse;
    mouse_get_state(&mouse);
    bool hoverd = mouse.x >= x && mouse.x < x + width && mouse.y >= y && mouse.y < y + height;
    graphics_rectangle(x, y, width, height, color);
    int text_width = 0;
    int character_count = 0;
    
    for (const char* character = text; *character != '\0'; character++) {
        character_count++;
    }

    if (character_count > 1) {
        text_width = character_count * 10 + (character_count - 1) * 4;
    }
    else if (character_count == 1) {
        text_width = 10;
    }

    int text_x = x + (width - text_width) / 2;
    int text_y = y + (height - 10) / 2;
    font_draw_text(text_x, text_y, text, text_color);
    return hoverd && mouse_left_clicked();
}

Window ui_create_window(int x, int y, int width, int height, const char* title,uint32_t background_color, uint32_t title_bar_color, uint32_t title_color, WindowDrawCallback draw_content) {
    Window window;
    window.x = x;
    window.y = y;
    window.width = width;
    window.height = height;
    window.title = title;
    window.dragging = false;
    window.drag_offset_x = 0;
    window.drag_offset_y = 0;
    window.background_color = background_color;
    window.title_bar_color = title_bar_color;
    window.title_color = title_color;
    window.draw_content = draw_content;
    return window;
}

void ui_register_window(Window* window) {
    if (window == nullptr) {
        return;
    }

    if (window_count >= MAX_WINDOWS) {
        return;
    }

    windows[window_count] = window;
    window_count++;
}

static Window* ui_get_window_at(int mouse_x, int mouse_y) {
    for (int i = window_count - 1; i >= 0; i--) {
        Window* window = windows[i];

        if (mouse_x >= window->x && mouse_x < window->x + window->width && mouse_y >= window->y && mouse_y < window->y + window->height) {
            return window;
        }
    }

    return nullptr;
}

static void ui_bring_to_front(Window* window) {
    if (window == nullptr) {
        return;
    }

    int index = -1;

    for (int i = 0; i < window_count; i++) {
        if (windows[i] == window) {
            index = i;
            break;
        }
    }

    if (index == -1 || index == window_count - 1) {
        return;
    }

    for (int i = index; i < window_count - 1; i++) {
        windows[i] = windows[i + 1];
    }

    windows[window_count - 1] = window;
}

void ui_update_window(Window* window, bool active) {
    if (window == nullptr) {
        return;
    }

    MouseState mouse;
    mouse_get_state(&mouse);
    bool mouse_left_down = mouse_left_pressed();
    bool mouse_in_title_bar = mouse.x >= window->x && mouse.x < window->x + window->width && mouse.y >= window->y && mouse.y < window->y + WINDOW_TITLE_BAR_HEIGHT;

    if (active) {
        if (!window->dragging && mouse_left_down && mouse_in_title_bar) {
            window->dragging = true;
            window->drag_offset_x = mouse.x - window->x;
            window->drag_offset_y = mouse.y - window->y;
        }
    }

    if (window->dragging) {
        if (mouse_left_down) {
            window->x = mouse.x - window->drag_offset_x;
            window->y = mouse.y - window->drag_offset_y;
        }
        else {
            window->dragging = false;
        }
    }

    int desktop_width = graphics_width();
    int desktop_height_value = desktop_height();

    if (window->x < 0) {
        window->x = 0;
    }

    if (window->y < 0) {
        window->y = 0;
    }

    if (window->x + window->width > desktop_width) {
        window->x = desktop_width - window->width;
    }

    if (window->y + window->height > desktop_height_value) {
        window->y = desktop_height_value - window->height;
    }

    // Safety if window is bigger than desktop
    if (window->x < 0) {
        window->x = 0;
    }

    if (window->y < 0) {
        window->y = 0;
    }
}

void ui_update_windows() {
    MouseState mouse;
    mouse_get_state(&mouse);

    if (focused_window != nullptr && focused_window->dragging) {
        ui_update_window(focused_window, true);
        return;
    }

    Window* active_window = ui_get_window_at(mouse.x, mouse.y);

    if (active_window != nullptr && mouse_left_clicked()) {
        focused_window = active_window;
        ui_bring_to_front(active_window);
        ui_update_window(focused_window, true);
        return;
    }

    if (focused_window != nullptr) {
        ui_update_window(focused_window, true);
    }
}

void ui_draw_window(Window* window) {
    if (window == nullptr) {
        return;
    }

    graphics_rectangle(window->x, window->y, window->width, window->height, window->background_color);
    graphics_rectangle(window->x, window->y, window->width, WINDOW_TITLE_BAR_HEIGHT, window->title_bar_color);

    if (window->title != nullptr) {
        font_draw_text(window->x + 10, window->y + 8, window->title, window->title_color);
    }
}

void ui_draw_windows() {
    for (int i = 0; i < window_count; i++) {
        Window* window = windows[i];

        if (window == nullptr) {
            continue;
        }

        ui_draw_window(window);

        if (window->draw_content != nullptr) {
            window->draw_content(window);
        }
    }
}

int ui_window_content_x(const Window* window) {
    return window->x;
}

int ui_window_content_y(const Window* window) {
    return window->y + WINDOW_TITLE_BAR_HEIGHT;
}

int ui_window_content_width(const Window* window) {
    return window->width;
}

int ui_window_content_height(const Window* window) {
    return window->height - WINDOW_TITLE_BAR_HEIGHT;
}