/**
 * @file main/util/utils.c
 * @brief General utility functions.
 */

#include "utils.h"
#include "error.h"
#include <stdio.h>

void bytes_to_hex(const uint8_t* data, size_t len, char* out, size_t out_size) {
    ASSERT_OR_DIE(data && len > 0, "invalid data");
    ASSERT_OR_DIE(out, "null output buffer");
    ASSERT_OR_DIE(out_size >= len * 2 + 1, "hex buffer too small");
    for (size_t i = 0; i < len; i++) {
        int res = snprintf(out + i * 2, 3, "%02x", data[i]);
        ASSERT_OR_DIE(res > 0 && (size_t)res < 3, "hex formatting failed");
    }
    out[len * 2] = '\0';
}
