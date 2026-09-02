/**
 * @file main/ui/log.c
 * @brief Action log ring buffer and state screen.
 */

#include "log.h"
#include "hal.h"
#include "mnemonic_view.h"
#include "ui_internal.h"
#include "util/error.h"
#include <stdarg.h>
#include <stdio.h>

/* -- Action log ring buffer ------------------------------------------- */
#define LOG_MAX 32
#define LOG_LEN 96
static char log_buf[LOG_MAX][LOG_LEN];
static int  log_head  = 0; // next write position
static int  log_count = 0; // total entries (capped at LOG_MAX)

void ui_log_add(const char* fmt, ...) {
    // Prefix an elapsed-time stamp (uptime HH:MM:SS) to each entry.
    uint32_t t = lv_tick_get();
    unsigned h = (unsigned)(t / 3600000u);
    unsigned m = (unsigned)((t / 60000u) % 60u);
    unsigned s = (unsigned)((t / 1000u) % 60u);

    int prefix = snprintf(log_buf[log_head], LOG_LEN, "[%02u:%02u:%02u] ", h, m, s);

    va_list args;
    va_start(args, fmt);
    vsnprintf(log_buf[log_head] + prefix, LOG_LEN - prefix, fmt, args);
    va_end(args);

    log_head = (log_head + 1) % LOG_MAX;
    if (log_count < LOG_MAX) log_count++;
}

/* -- State screen ----------------------------------------------------- */
void ui_show_state(ui_cb_t on_back, const char* mnemonic_words) {
    ASSERT_OR_DIE(on_back, "null on_back");

    lv_obj_t* s = ui_make_screen();
    ui_add_title(s, "State & Log");

    // mnemonic display: numbered grid, or a placeholder if none yet
    lv_obj_t* mn_area;
    if (mnemonic_words && mnemonic_words[0]) {
        mn_area = ui_mnemonic_view_create(s);
        ui_mnemonic_view_set_words(mn_area, mnemonic_words);
    } else {
        mn_area = lv_label_create(s);
        lv_label_set_text(mn_area, "(no mnemonic yet)");
        lv_obj_set_style_text_color(mn_area, lv_color_hex(0x888888), 0);
        lv_obj_set_style_text_font(mn_area, ui_font(18), 0);
    }
    lv_obj_align(mn_area, LV_ALIGN_TOP_MID, 0, ui_scale(48));
    lv_obj_update_layout(mn_area);

    // Cap tall grids (24 words) so the log below always has room; allow scroll.
    bool       mn_scrollable = false;
    lv_coord_t mn_h          = lv_obj_get_height(mn_area);
    if (mnemonic_words && mnemonic_words[0] && mn_h > ui_scale(150)) {
        lv_obj_set_height(mn_area, ui_scale(150));
        lv_obj_add_flag(mn_area, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_scroll_dir(mn_area, LV_DIR_VER);
        mn_scrollable = true;
    }
    if (mn_scrollable && ui_small_screen()) {
        lv_obj_t* arrows = ui_add_scroll_arrows(s, mn_area, ui_scale(30));
        lv_obj_align_to(arrows, mn_area, LV_ALIGN_OUT_RIGHT_MID, ui_scale(4), 0);
    }

    // log entries (oldest first, top to bottom)
    lv_obj_t* log_cont = lv_obj_create(s);
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
        lv_obj_set_style_text_font(entry, ui_font(14), 0);
        lv_label_set_long_mode(entry, LV_LABEL_LONG_WRAP);
        lv_obj_set_width(entry, ui_scale(420));
    }

    // Fit the log between the mnemonic area and the back button.
    lv_coord_t mn_bottom = lv_obj_get_y(mn_area) + lv_obj_get_height(mn_area);
    lv_coord_t log_top   = mn_bottom + ui_scale(8);
    lv_coord_t log_h =
        (LV_VER_RES - ui_scale(10) - ui_scale(44)) - ui_scale(8) - log_top; /* above back button */
    if (log_h < 40) log_h = 40;
    lv_obj_set_size(log_cont, ui_scale(440), log_h);
    lv_obj_align(log_cont, LV_ALIGN_TOP_MID, 0, log_top);

    if (!hal_touch_available()) {
        lv_obj_t* arrows = ui_add_scroll_arrows(s, log_cont, ui_scale(24));
        lv_obj_align_to(arrows, log_cont, LV_ALIGN_OUT_RIGHT_MID, ui_scale(4), 0);
    }

    // back button
    ui_add_btn(s, "Back", on_back, UI_BTN_SIZE_MED, LV_ALIGN_BOTTOM_MID, 0, -10);

    ui_swap_screen(s);
}
