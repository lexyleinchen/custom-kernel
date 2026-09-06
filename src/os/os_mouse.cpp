#include "os_mouse.h"
#include "graphics.h"

#include "../kernel/mouse.h"

static const char* cursor[] = {
    "##..............",
    "###.............",
    "####............",
    "##W##...........",
    "##WW##..........",
    "##WWW##.........",
    "##WWWW##........",
    "##WWWWW##.......",
    "##WWWWWW##......",
    "##WWWWWWW##.....",
    "##WWWWWWWW##....",
    "##WWWWWWWWW##...",
    "##WWWWWWWWWW##..",
    "##WWWWWWWWWWW##.",
    "##WWWWWWWWWWWW##",
    "##WWWWWWWWWWWW##",
    "##WWWWWWWWWWWW##",
    "##WWWWWWWWW####.",
    "##WWWWWW####....",
    "##WWW####.......",
    "######..........",
    "###............."
};

void mouse_draw(void) {
    MouseState state;
    mouse_get_state(&state);

    int x = state.x;
    int y = state.y;

    for (int roW = 0; roW < 22; roW++){
        for (int column = 0; column < 16; column++) {
            uint32_t pixel = 0;
            if (cursor[roW][column] == '#') {
                pixel = 0xFF000000;
            }
            else if (cursor[roW][column] == 'W') {
                pixel = 0xFFFFFFFF;
            }
            else {
                continue;
            }

            graphics_rectangle(x + column, y + roW, 1, 1, pixel);
        }
    }
}