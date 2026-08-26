/**
 * @file platform/linux/hal_linux.c
 * @brief Linux HAL implementation
 */

#include "hal.h"
#include "util/error.h"
#include <stdio.h>

const char* hal_get_random_source() { return "/dev/urandom"; }

void hal_get_random(uint8_t* buf, size_t len) {
    ASSERT_OR_DIE(buf && len > 0, "hal_get_random: invalid buffer");

    FILE* f = fopen("/dev/urandom", "rb");
    ASSERT_OR_DIE(f, "hal_get_random: failed to open /dev/urandom");
    size_t n = fread(buf, 1, len, f);
    ASSERT_OR_DIE(n == len, "hal_get_random: short read from /dev/urandom");
    fclose(f);
}
