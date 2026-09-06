#ifndef UI_H
#define UI_H

#include <stdint.h>

bool graphics_button(int x, int y, int width, int height, uint32_t color, const char* text, uint32_t text_color);

#endif // UI_H