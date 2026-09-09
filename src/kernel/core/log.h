#ifndef LOG_H
#define LOG_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void log_init(void);

void kernel_log(const char* text, ...);

int log_count(void);

const char* log_get_line(int index);

#ifdef __cplusplus
}
#endif

#endif // LOG_H