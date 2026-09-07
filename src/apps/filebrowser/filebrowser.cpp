#include "filebrowser.h"

#include "../../os/app_registry.h"
#include "../../os/graphics.h"
#include "../../os/font.h"

namespace filebrowser {
    void init() {

    }

    void draw() {
        font_draw_text(100, 1000, "Filebrowser", 0xFFFFFFFF);
    }
}

REGISTER_APP(filebrowser);