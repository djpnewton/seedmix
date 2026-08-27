/**
 * @file main/util/log.c
 * @brief Logging implementation - level/line prefixed output to stderr.
 */

#include "log.h"
#include <stdarg.h>
#include <stdio.h>

void log_msg(const char* level, const char* file, int line, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "[%-5s] %s:%d  ", level, file, line);
    vfprintf(stderr, fmt, args);
    fputc('\n', stderr);
    va_end(args);
}
