#include "calculator.h"

#include "../../os/app_registry.h"
#include "../../os/graphics.h"
#include "../../os/font.h"

namespace calculator {
    void init() {

    }

    void draw() {
        font_draw_text(100, 500, "Calculator", 0xFFFFFFFF);
    }
}

REGISTER_APP(calculator);