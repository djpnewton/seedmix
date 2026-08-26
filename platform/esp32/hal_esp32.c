/**
 * @file platform/esp32/hal_esp32.c
 * @brief ESP32 HAL implementation
 */

#include "hal.h"
#include "util/error.h"

const char* hal_get_random_source() { ASSERT_OR_DIE(0, "Not yet implemented"); }

void hal_get_random(uint8_t* buf, size_t len) {
    ASSERT_OR_DIE(buf && len > 0, "hal_get_random: invalid buffer");

    ASSERT_OR_DIE(0, "Not yet implemented");
}
