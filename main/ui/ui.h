/**
 * @file main/ui/ui.h
 * @brief UI screen creation and interaction API.
 */

#ifndef UI_H
#define UI_H

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Build all UI screens and objects.
 */
void ui_create(void);

/**
 * @brief Show the generated mnemonic on screen.  `on_done` is called when
 *        the user presses "Done" to dismiss the screen.
 */
void ui_show_mnemonic(const char *words, void (*on_done)(void));

/**
 * @brief Return to the main screen.
 */
void ui_show_main(void);

/**
 * @brief Register a callback for the "Generate" button.
 *
 * @param cb  LVGL v9 event callback: void cb(lv_event_t *e).
 */
void ui_on_generate(lv_event_cb_t cb);

#ifdef __cplusplus
}
#endif

#endif /* UI_H */
