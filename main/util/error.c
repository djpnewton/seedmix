/**
 * @file main/util/error.c
 * @brief Fatal error screen rendered via LVGL.
 *
 * If a display is active, renders file/line/message and spins forever.
 * Otherwise falls back to stderr + exit.
 */

#include "error.h"
#include "log.h"
#include "lvgl.h"
#include "ui/ui.h"
#include "ui/ui_internal.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* -- LVGL error screen ------------------------------------------------ */
static void error_screen(const char* file, int line, const char* fmt, va_list args) {
    char buf[256];
    vsnprintf(buf, sizeof(buf), fmt, args);
    LOG_ERROR("*** FATAL - %s", buf);

    if (!lv_display_get_default()) {
        LOG_ERROR("No display available - exiting");
        exit(1);
    }

    // make error screen
    lv_obj_t* scr = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x8B0000), 0); // dark red

    lv_obj_t* title = lv_label_create(scr);
    lv_label_set_text(title, "FATAL ERROR");
    lv_obj_set_style_text_color(title, lv_color_white(), 0);
    lv_obj_set_style_text_font(title, ui_font(48), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, ui_scale(30));

    char loc[64];
    snprintf(loc, sizeof(loc), "%s:%d", file, line);
    lv_obj_t* loc_label = lv_label_create(scr);
    lv_label_set_text(loc_label, loc);
    lv_obj_set_style_text_color(loc_label, lv_color_hex(0xFF8888), 0);
    lv_obj_set_style_text_font(loc_label, ui_font(20), 0);
    lv_obj_align(loc_label, LV_ALIGN_CENTER, 0, ui_scale(-20));

    lv_obj_t* msg = lv_label_create(scr);
    lv_label_set_text(msg, buf);
    lv_obj_set_style_text_color(msg, lv_color_white(), 0);
    lv_obj_set_style_text_font(msg, ui_font(20), 0);
    lv_obj_set_width(msg, ui_scale(440));
    lv_label_set_long_mode(msg, LV_LABEL_LONG_WRAP);
    lv_obj_align(msg, LV_ALIGN_CENTER, 0, ui_scale(50));

    lv_obj_t* footer = lv_label_create(scr);
    lv_label_set_text(footer, "Restart device to recover");
    lv_obj_set_style_text_color(footer, lv_color_hex(0x888888), 0);
    lv_obj_set_style_text_font(footer, ui_font(14), 0);
    lv_obj_align(footer, LV_ALIGN_BOTTOM_MID, 0, ui_scale(-10));

    // Render immediately: fatal_handler() may be running inside an LVGL
    // timer/event callback, so we cannot rely on the main loop (or a
    // deferred lv_async_call) to paint the screen for us.
    lv_scr_load(scr);
    lv_refr_now(NULL);
}

void fatal_handler(const char* file, int line, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    error_screen(file, line, fmt, args);
    va_end(args);

#ifdef ESP_PLATFORM
    while (1) {
        ui_delay_ms(5);
    }
#else
    sleep(3);
    exit(1);
#endif
}
