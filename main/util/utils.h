/**
 * @file main/util/utils.h
 * @brief General utility functions.
 */

#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Format bytes as lowercase hex.
 * @param data      Input bytes.
 * @param len       Number of bytes.
 * @param out       Output buffer (at least len * 2 + 1 chars).
 * @param out_size  Size of `out` in bytes.
 */
void bytes_to_hex(const uint8_t* data, size_t len, char* out, size_t out_size);

#ifdef __cplusplus
}
#endif

#endif /* UTILS_H */
