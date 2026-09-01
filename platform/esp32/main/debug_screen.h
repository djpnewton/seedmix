/**
 * @file debug_screen.h
 * @brief Hardware debug screen for the seedmix ESP32 firmware.
 */

#ifndef SEEDMIX_ESP32_DEBUG_SCREEN_H
#define SEEDMIX_ESP32_DEBUG_SCREEN_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Create and show the debug screen.
 *
 * Shows the resolved hardware configuration plus a live button test: while a
 * physical button is held, a dedicated "pressed" screen is shown, and the
 * debug screen is restored on release.
 *
 * Must be called after display_init() and buttons_init().
 */
void debug_screen_init(void);

#ifdef __cplusplus
}
#endif

#endif /* SEEDMIX_ESP32_DEBUG_SCREEN_H */
