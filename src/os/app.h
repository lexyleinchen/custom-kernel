#ifndef APP_H
#define APP_H

typedef void (*AppInitFunc)();

typedef void (*AppDrawFunc)();

struct App {
    const char* name;
    AppInitFunc init;
    AppDrawFunc draw;
};

#endif // APP_H