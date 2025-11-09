#ifndef LOG_H
#define LOG_H

#include <stdarg.h>

void log_init(void);
void log_fake_init(void);
void log_clear(void);
void lprintf(const char *format, ...);

#endif // LOG_H
