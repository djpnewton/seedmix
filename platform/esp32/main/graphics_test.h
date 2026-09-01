/**
 * @file graphics_test.h
 * @brief Graphics test screen for verifying panel rendering.
 */

#ifndef SEEDMIX_ESP32_GRAPHICS_TEST_H
#define SEEDMIX_ESP32_GRAPHICS_TEST_H

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Create the graphics test screen (content only; not loaded).
 *
 * Draws color swatches, text, an arc and a rounded rectangle so rendering
 * issues (color order, gaps, corruption) can be spotted at a glance.
 */
lv_obj_t* graphics_test_create(void);

#ifdef __cplusplus
}
#endif

#endif /* SEEDMIX_ESP32_GRAPHICS_TEST_H */
