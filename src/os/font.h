#ifndef FONT_H
#define FONT_H

#include <stdint.h>

void font_draw_char(int x, int y, char c, uint32_t color);

void font_draw_text(int x, int y, const char* text, uint32_t color);

#endif // FONT_HF