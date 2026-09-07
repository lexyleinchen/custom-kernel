#ifndef APP_REGISTRY_H
#define APP_REGISTRY_H

#include "app.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const App __app_start[];

extern const App __app_end[];

#ifdef __cplusplus
}
#endif

static inline int app_count() {
    return __app_end - __app_start;
}

static inline const App* app_get(int index) {
    return &__app_start[index];
}

#define REGISTER_APP(name) \
    __attribute__((used, section(".apps"))) \
    static const App __app_##name = { \
        #name, \
        name::init, \
        name::draw \
    };

#endif // APP_REGISTRY_H