/**
 * @file main/ui/log.h
 * @brief Action log ring buffer and state screen.
 */

#ifndef UI_LOG_H
#define UI_LOG_H

#include "lvgl.h"
#include "ui.h"
#include "util/compiler.h"

#ifdef __cplusplus
extern "C" {
#endif

void ui_log_add(const char* fmt, ...) PRINTF_LIKE(1, 2);
void ui_show_state(ui_cb_t on_back, const char* mnemonic_words);

#ifdef __cplusplus
}
#endif

#endif /* UI_LOG_H */
