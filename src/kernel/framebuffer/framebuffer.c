#include "framebuffer.h"

static Framebuffer framebuffer;

void framebuffer_init(Framebuffer fb) {
    framebuffer = fb;
}

Framebuffer framebuffer_get(void) {
    return framebuffer;
}