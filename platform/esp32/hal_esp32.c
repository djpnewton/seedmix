/**
 * @file platform/esp32/hal_esp32.c
 * @brief ESP32 HAL implementation
 */

#include "hal.h"
#include "util/error.h"
#include "util/utils.h"
#include <stdlib.h>
#include <string.h>

const char* hal_get_random_source() { ASSERT_OR_DIE(0, "Not yet implemented"); }

void hal_get_random(uint8_t* buf, size_t len) {
    ASSERT_OR_DIE(buf && len > 0, "hal_get_random: invalid buffer");

    ASSERT_OR_DIE(0, "Not yet implemented");
}

bool hal_camera_available(void) { return false; }

bool hal_camera_capture(hal_camera_frame_t* frame) {
    ASSERT_OR_DIE(frame, "hal_camera_capture: null frame");
    memset(frame, 0, sizeof(*frame));
    return false; /* camera not implemented on ESP32 yet */
}

void hal_camera_frame_free(hal_camera_frame_t* frame) {
    if (!frame) return;
    if (frame->data) {
        secure_memzero(frame->data, frame->size);
        free(frame->data);
    }
    memset(frame, 0, sizeof(*frame));
}
