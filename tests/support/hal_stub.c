/**
 * @file tests/support/hal_stub.c
 * @brief Deterministic HAL random stub for unit tests.
 *
 * The real implementations live in platform/<platform>/hal_<platform>.c and
 * use platform entropy sources (/dev/urandom on Linux, hardware TRNG on
 * ESP32).  Tests link this stub so entropy is reproducible.
 */

#include "hal.h"

static uint32_t s_state = 0x12345678u;

static uint32_t xorshift32(void) {
    uint32_t x = s_state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    s_state = x;
    return x;
}

const char* hal_get_random_source(void) { return "test-deterministic"; }

void hal_get_random(uint8_t* buf, size_t len) {
    if (!buf || len == 0) return;
    for (size_t i = 0; i < len; i++) {
        buf[i] = (uint8_t)(xorshift32() & 0xFFu);
    }
}
