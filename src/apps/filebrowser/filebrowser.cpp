#include "filebrowser.h"

#include "../../os/app_registry.h"
#include "../../os/font.h"
#include "../../os/ui.h"

namespace filebrowser {
    static Window window;

    static void draw_content(Window* window) {
        int content_x = ui_window_content_x(window);
        int content_y = ui_window_content_y(window);
        int content_width = ui_window_content_width(window);
        int content_height = ui_window_content_height(window);

        font_draw_text(content_x + 20, content_y + 20, "Filebrowser", 0xFFFFFFFF);
    }

    void init() {
        window = ui_create_window(60, 60, 800, 450, "Filebrowser", 0xFF000000, 0xFF808080, 0xFFFFFFFF, nullptr, draw_content);
        ui_register_window(&window);
    }
}

REGISTER_APP(filebrowser);