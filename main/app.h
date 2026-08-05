/**
 * @file main/app.h
 * @brief Shared application interface - called by platform entry points.
 */

#ifndef APP_H
#define APP_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the application UI and subsystems.
 *
 * Called once by the platform entry point after LVGL is ready.
 */
void app_init(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_H */
