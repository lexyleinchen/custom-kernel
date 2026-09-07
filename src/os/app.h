#ifndef APP_H
#define APP_H

typedef void (*AppInitFunc)();

struct App {
    const char* name;
    AppInitFunc init;
};

#endif // APP_H