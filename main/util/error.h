/**
 * @file main/util/error.h
 * @brief Fatal error handling - renders an LVGL error screen with file/line.
 */

#ifndef ERROR_H
#define ERROR_H

#include <stdarg.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Show a fatal error screen via LVGL (or stderr fallback) and halt.
 *
 * If a display is active, renders a red screen with file, line, and message,
 * then spins forever.  Otherwise prints to stderr and exits.
 */
void fatal_handler(const char* file, int line, const char* fmt, ...);

/**
 * @brief Halt with file, line, and formatted message on the LVGL error screen.
 */
#define SHORT_FILE (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define FATAL(fmt, ...) fatal_handler(SHORT_FILE, __LINE__, fmt, ##__VA_ARGS__)

/**
 * @brief If `expr` is false (0), call FATAL with the given format string.
 */
#define ASSERT_OR_DIE(expr, fmt, ...) do { \
    if (!(expr)) { FATAL(fmt, ##__VA_ARGS__); } \
} while(0)

#ifdef __cplusplus
}
#endif

#endif /* ERROR_H */
