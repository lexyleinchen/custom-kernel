#include "terminal.h"

#include "../../os/app_registry.h"
#include "../../os/font.h"
#include "../../os/ui.h"

#include "../../kernel/log.h"

namespace terminal {
    static Window window;
    static int scroll_line = 0;
    static int scroll_direction = 1;
    static int scroll_delay = 0;
    static int end_dalay = 0;
    static int terminal_width = 800;
    static int terminal_height = 450;

    #define SCROLL_SPEED 20
    #define END_WAIT 240 // 240 frames = ~4 second when 60fps

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

        int max_lines = (content_height - 15) / 16;
        int max_scroll = count - max_lines;
        
        if (max_lines < 1) {
            max_scroll = 1;
        }

        if (max_scroll < 0) {
            max_scroll = 0;
        }

        if (max_scroll > 0) {
            if (end_dalay > 0) {
                end_dalay--;
            }
            else {
                scroll_delay++;

                if (scroll_delay >= 20) {
                    scroll_delay = 0;
                    scroll_line += scroll_direction;

                    if (scroll_line >= max_scroll) {
                        scroll_line = max_scroll;
                        scroll_direction = -1;
                        end_dalay = END_WAIT;
                    }

                    if (scroll_line <= 0) {
                        scroll_line = 0;
                        scroll_direction = 1;
                        end_dalay = END_WAIT;
                    }
                }
            }
        }
        else {
            scroll_line = 0;
            scroll_delay = 0;
            end_dalay = 0;
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
    }

    void init() {
        scroll_line = 0;
        scroll_direction = 1;
        scroll_delay = 0;
        end_dalay = 0;

        window = ui_create_window(10, 10, terminal_width, terminal_height, "Terminal", 0xFF000000, 0xFF808080, 0xFFFFFFFF, draw_content);
        ui_register_window(&window);
    }
}

REGISTER_APP(terminal);