/**
 * @file main/ui/log.c
 * @brief Action log ring buffer and state screen.
 */

#include "log.h"
#include "ui_internal.h"
#include "util/error.h"
#include <stdarg.h>
#include <stdio.h>

/* -- Action log ring buffer ------------------------------------------- */
#define LOG_MAX   32
#define LOG_LEN   96
static char  log_buf[LOG_MAX][LOG_LEN];
static int   log_head = 0;  // next write position
static int   log_count = 0; // total entries (capped at LOG_MAX)

void ui_log_add(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vsnprintf(log_buf[log_head], LOG_LEN, fmt, args);
    va_end(args);
    log_head = (log_head + 1) % LOG_MAX;
    if (log_count < LOG_MAX) log_count++;
}

/* -- State screen ----------------------------------------------------- */
void ui_show_state(ui_cb_t on_back, const char* mnemonic_words) {
    ASSERT_OR_DIE(on_back, "null on_back");

    lv_obj_t* s = ui_make_screen();
    ui_add_title(s, "State & Log");

    // mnemonic display
    lv_obj_t* ml = lv_label_create(s);
    if (mnemonic_words && mnemonic_words[0]) {
        lv_label_set_text(ml, mnemonic_words);
    } else {
        lv_label_set_text(ml, "(no mnemonic yet)");
    }
    lv_obj_set_style_text_color(ml, lv_color_white(), 0);
    lv_obj_set_style_text_font(ml, &lv_font_montserrat_18, 0);
    lv_obj_set_style_text_align(ml, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(ml, 440);
    lv_label_set_long_mode(ml, LV_LABEL_LONG_WRAP);
    lv_obj_align(ml, LV_ALIGN_TOP_MID, 0, 35);

    // log entries (oldest first, top to bottom)
    lv_obj_t* log_cont = lv_obj_create(s);
    lv_obj_set_size(log_cont, 440, 200);
    lv_obj_align(log_cont, LV_ALIGN_BOTTOM_MID, 0, -65);
    lv_obj_set_style_bg_color(log_cont, lv_color_hex(0x111111), 0);
    lv_obj_set_style_border_width(log_cont, 0, 0);
    lv_obj_set_flex_flow(log_cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(log_cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_scroll_dir(log_cont, LV_DIR_VER);

    int start = (log_count < LOG_MAX) ? 0 : log_head;
    for (int i = 0; i < log_count; i++) {
        int idx = (start + i) % LOG_MAX;
        if (!log_buf[idx][0]) continue;
        lv_obj_t* entry = lv_label_create(log_cont);
        lv_label_set_text(entry, log_buf[idx]);
        lv_obj_set_style_text_color(entry, lv_color_hex(0xAAAAAA), 0);
        lv_obj_set_style_text_font(entry, &lv_font_montserrat_14, 0);
    }

    // back button
    lv_obj_t* back = lv_button_create(s);
    lv_obj_set_size(back, 160, 50);
    lv_obj_align(back, LV_ALIGN_BOTTOM_MID, 0, -10);
    union { ui_cb_t fn; void* vp; } u = { .fn = on_back };
    lv_obj_add_event_cb(back, ui_btn_invoke, LV_EVENT_CLICKED, u.vp);
    lv_obj_t* bl = lv_label_create(back);
    lv_label_set_text(bl, "Back");
    lv_obj_set_style_text_font(bl, &lv_font_montserrat_24, 0);
    lv_obj_center(bl);

    ui_swap_screen(s);
}
