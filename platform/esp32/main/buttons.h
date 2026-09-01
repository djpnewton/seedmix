/**
 * @file buttons.h
 * @brief Physical GPIO buttons - raw hardware layer.
 */

#ifndef SEEDMIX_ESP32_BUTTONS_H
#define SEEDMIX_ESP32_BUTTONS_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Configure the physical button GPIOs. Call once after boot. */
void buttons_init(void);

/** @brief Number of configured buttons (0 if disabled). */
int buttons_count(void);

/** @brief True while button `idx` is physically held (raw, no debounce). */
bool button_is_pressed(int idx);

/** @brief True while any button is held. */
bool buttons_any_pressed(void);

#ifdef __cplusplus
}
#endif

#endif /* SEEDMIX_ESP32_BUTTONS_H */
