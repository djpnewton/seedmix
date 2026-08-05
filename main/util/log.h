/**
 * @file main/util/log.h
 * @brief Logging macros - file/line prefixed output to stderr.
 */

#ifndef LOG_H
#define LOG_H

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LOG_INFO(fmt, ...)  fprintf(stderr, "[INFO]  %s:%d  " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...)  fprintf(stderr, "[WARN]  %s:%d  " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) fprintf(stderr, "[ERROR] %s:%d  " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif /* LOG_H */
