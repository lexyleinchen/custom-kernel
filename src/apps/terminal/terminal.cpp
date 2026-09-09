#include "terminal.h"

#include "../../os/app_registry.h"
#include "../../os/font.h"
#include "../../os/ui.h"

#include "../../kernel/core/log.h"

namespace terminal {
    static Window window;
    static int scroll_line = 0;
    static int terminal_width = 800;
    static int terminal_height = 450;
    static int previous_log_count = 0;
    static int previous_max_scroll = 0;

    static void draw_content(Window* window) {
        int content_x = ui_window_content_x(window);
        int content_y = ui_window_content_y(window);
        int content_width = ui_window_content_width(window);
        int content_height = ui_window_content_height(window);
        int text_x = content_x + 15;
        int text_y = content_y + 15;
        int count = log_count();

        if (count <= 0) {
            return;
        }

        int scrollbar_x = content_x + content_width - 10;
        int max_lines = (content_height - 15) / 16;
        
        if (max_lines < 1) {
            max_lines = 1;
        }

        int max_scroll = count - max_lines;

        if (max_scroll < 0) {
            max_scroll = 0;
        }

        if (scroll_line < 0) {
            scroll_line = 0;
        }

        if (scroll_line > max_scroll) {
            scroll_line = max_scroll;
        }

        int y = text_y;
        
        for (int i = 0; i < max_lines; i++) {
            int index = scroll_line + i;

            if (index >= count) {
                break;
            }

            const char* text = log_get_line(index);
            int x = text_x;
            
            while (*text != '\0') {
                if (x + 10 >= content_x + content_width - 10) {
                    break;
                }

                font_draw_char(x, y, *text, 0xFFFFFFFF);
                x += 14;
                text++;
            }

            y += 16;
        }

        draw_scrollbar(scrollbar_x, content_y, 10, content_height, count, max_lines, scroll_line);
    }

    static void update_content(Window* window) {
        int content_x = ui_window_content_x(window);
        int content_y = ui_window_content_y(window);
        int content_width = ui_window_content_width(window);
        int content_height = ui_window_content_height(window);
        int max_lines = (content_height - 15) / 16;

        if (max_lines < 1) {
            max_lines = 1;
        }

        int count = log_count();
        int max_scroll = count - max_lines;

        if (max_scroll < 0) {
            max_scroll = 0;
        }

        bool was_at_bottom = scroll_line >= previous_max_scroll;

        if ((count > previous_log_count) && was_at_bottom) {
            scroll_line = max_scroll;
        }

        previous_log_count = count;
        previous_max_scroll = max_scroll;

        if (scroll_line < 0) {
            scroll_line = 0;
        }

        if (scroll_line > max_scroll) {
            scroll_line = max_scroll;
        }

        update_scrollbar(window, content_x + content_width - 10, content_y, 10, content_height, count, max_lines, scroll_line);
    }

    void init() {
        window = ui_create_window(10, 10, terminal_width, terminal_height, "Terminal", 0xFF000000, 0xFF808080, 0xFFFFFFFF, update_content, draw_content);
        ui_register_window(&window);
        int content_height = ui_window_content_height(&window);
        int max_lines = (content_height - 15) / 16;

        if (max_lines < 1) {
            max_lines = 1;
        }

        int count = log_count();
        int max_scroll = count - max_lines;

        if (max_scroll < 0) {
            max_scroll = 0;
        }

        scroll_line = max_scroll;
        previous_log_count = count;
        previous_max_scroll = max_scroll;
    }
}

REGISTER_APP(terminal);