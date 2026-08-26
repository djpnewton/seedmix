/**
 * @file main/ui/ui.h
 * @brief UI screens for the mnemonic wallet tool.
 */

#ifndef UI_H
#define UI_H

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*ui_cb_t)(void);

typedef enum {
    MNEMONIC_TYPE_GENERATED,
    MNEMONIC_TYPE_ENTERED,
    MNEMONIC_TYPE_MERGED,
    MNEMONIC_TYPE_FINAL,
} mnemonic_type_t;

void ui_show_main(lv_event_cb_t on_new_wallet, lv_event_cb_t on_test_error);
void ui_show_word_count(ui_cb_t on_12, ui_cb_t on_24);
void ui_show_source(ui_cb_t on_generate, ui_cb_t on_enter, ui_cb_t on_scan_qr, ui_cb_t on_state,
                    ui_cb_t on_finish, bool is_additional);

/** @deprecated Use ui_word_entry_begin from word_entry.h instead. */
void        ui_show_enter_words(ui_cb_t on_ok);
const char* ui_get_entered_words(void);

void ui_show_mnemonic(const char* words, mnemonic_type_t type, ui_cb_t on_ok);
void ui_show_merge_process(const char* current_words, const char* current_entropy_hex,
                           const char* new_entropy_hex, const char* merged_entropy_hex,
                           const char* merged_words, ui_cb_t on_ok);
void ui_show_msg(const char* msg);
void ui_delay_ms(uint32_t ms);
void ui_go_main(void);

#ifdef __cplusplus
}
#endif

#endif /* UI_H */
