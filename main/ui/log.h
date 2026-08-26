/**
 * @file main/ui/log.h
 * @brief Action log ring buffer and state screen.
 */

#ifndef LOG_H
#define LOG_H

#include "lvgl.h"
#include "ui.h"

#ifdef __cplusplus
extern "C" {
#endif

void ui_log_add(const char* fmt, ...);
void ui_show_state(ui_cb_t on_back, const char* mnemonic_words);

#ifdef __cplusplus
}
#endif

#endif /* LOG_H */
