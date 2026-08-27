/**
 * @file main/util/log.h
 * @brief Logging macros - file/line prefixed output to stderr.
 */

#ifndef LOG_H
#define LOG_H

#include <string.h>

#include "compiler.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -- Compile-time verbosity --------------------------------------------
 * Raise LOG_LEVEL (e.g. via -DLOG_LEVEL=LOG_LEVEL_ERROR) to drop
 * lower-priority messages at compile time. */
#define LOG_LEVEL_DEBUG 0
#define LOG_LEVEL_INFO 1
#define LOG_LEVEL_WARN 2
#define LOG_LEVEL_ERROR 3

#ifndef LOG_LEVEL
#define LOG_LEVEL LOG_LEVEL_INFO
#endif

#define LOG_BASENAME (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

/** Core logger (do not call directly - use the LOG_* macros). */
void log_msg(const char* level, const char* file, int line, const char* fmt, ...) PRINTF_LIKE(4, 5);

#define LOG_ERROR(fmt, ...) log_msg("ERROR", LOG_BASENAME, __LINE__, fmt, ##__VA_ARGS__)

#if LOG_LEVEL <= LOG_LEVEL_WARN
#define LOG_WARN(fmt, ...) log_msg("WARN", LOG_BASENAME, __LINE__, fmt, ##__VA_ARGS__)
#else
#define LOG_WARN(fmt, ...) ((void)0)
#endif

#if LOG_LEVEL <= LOG_LEVEL_INFO
#define LOG_INFO(fmt, ...) log_msg("INFO", LOG_BASENAME, __LINE__, fmt, ##__VA_ARGS__)
#else
#define LOG_INFO(fmt, ...) ((void)0)
#endif

#if LOG_LEVEL <= LOG_LEVEL_DEBUG
#define LOG_DEBUG(fmt, ...) log_msg("DEBUG", LOG_BASENAME, __LINE__, fmt, ##__VA_ARGS__)
#else
#define LOG_DEBUG(fmt, ...) ((void)0)
#endif

#ifdef __cplusplus
}
#endif

#endif /* LOG_H */
