#include "terminal.h"

#include "../../os/app_registry.h"
#include "../../os/graphics.h"
#include "../../os/font.h"

#include "../../kernel/log.h"

namespace terminal {
    static const int TERMINAL_X = 10;
    static const int TERMINAL_Y = 10;
    static const int TITLE_BAR_HEIGHT = 25;
    static const int TEXT_X = TERMINAL_X + 15;
    static const int TEXT_Y = TERMINAL_Y + 40;

    static int scroll_line = 0;
    static int scroll_direction = 1;
    static int scroll_delay = 0;
    static int end_dalay = 0;

    #define SCROLL_SPEED 20
    #define END_WAIT 240 // 240 frames = ~4 second when 60fps

    void init() {
        scroll_line = 0;
        scroll_direction = 1;
        scroll_delay = 0;
        end_dalay = 0;
    }

    void draw() {
        int terminal_width = 800;
        int terminal_height = 450;

        graphics_rectangle(TERMINAL_X, TERMINAL_Y, terminal_width, terminal_height, 0xFF000000); // Draw the terminal background (black)
        graphics_rectangle(TERMINAL_X, TERMINAL_Y, terminal_width, TITLE_BAR_HEIGHT, 0xFF808080); // Draw the terminal titlebar (gray)
        font_draw_text(TERMINAL_X + 10, TERMINAL_Y + 8, "Terminal", 0xFFFFFFFF); // Draw the text "Terminal" (white)

        int count = log_count();

        if (count <= 0) {
            return;
        }

        int max_lines = (terminal_height - TITLE_BAR_HEIGHT - 15) / 16;

        if (max_lines < 1) {
            return;
        }

        int max_scroll = count - max_lines;

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

        int y = TEXT_Y;
        
        for (int i = 0; i < max_lines; i++) {
            int index = scroll_line + i;

            if (index >= count) {
                break;
            }

            const char* text = log_get_line(index);
            int x = TEXT_X;
            
            while (*text != '\0') {
                if (x + 10 >= TERMINAL_X + terminal_width - 10) {
                    break;
                }

                font_draw_char(x, y, *text, 0xFFFFFFFF);
                x += 14;
                text++;
            }

            y += 16;
        }
    }
}

REGISTER_APP(terminal);