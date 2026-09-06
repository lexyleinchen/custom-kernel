#include "ui.h"
#include "graphics.h"
#include "font.h"

#include "../kernel/mouse.h"

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