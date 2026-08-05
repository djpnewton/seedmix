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
#include <stdio.h>
#include <stdlib.h>

#ifndef ESP_PLATFORM
#include <SDL2/SDL.h>
#endif

/* -- LVGL error screen ------------------------------------------------ */
static void error_screen(const char *file, int line, const char *fmt, va_list args) {
    /* Format the log message */
    char buf[256];
    vsnprintf(buf, sizeof(buf), fmt, args);

    /* Always log to stderr */
    LOG_ERROR("*** FATAL - %s", buf);

    /* Try to show on screen if LVGL is initialized */
    lv_display_t *disp = lv_display_get_default();
    if (!disp) {
        exit(1);
    }

    /* Build error screen */
    lv_obj_t *scr = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x8B0000), 0); /* dark red */
    lv_scr_load(scr);

    /* Title */
    lv_obj_t *title = lv_label_create(scr);
    lv_label_set_text(title, "FATAL ERROR");
    lv_obj_set_style_text_color(title, lv_color_white(), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_48, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 30);

    /* Location */
    char loc[64];
    snprintf(loc, sizeof(loc), "%s:%d", file, line);
    lv_obj_t *loc_label = lv_label_create(scr);
    lv_label_set_text(loc_label, loc);
    lv_obj_set_style_text_color(loc_label, lv_color_hex(0xFF8888), 0);
    lv_obj_set_style_text_font(loc_label, &lv_font_montserrat_20, 0);
    lv_obj_align(loc_label, LV_ALIGN_CENTER, 0, -20);

    /* Message */
    lv_obj_t *msg = lv_label_create(scr);
    lv_label_set_text(msg, buf);
    lv_obj_set_style_text_color(msg, lv_color_white(), 0);
    lv_obj_set_style_text_font(msg, &lv_font_montserrat_20, 0);
    lv_obj_set_width(msg, 440);
    lv_label_set_long_mode(msg, LV_LABEL_LONG_WRAP);
    lv_obj_align(msg, LV_ALIGN_CENTER, 0, 50);

    /* Footer */
    lv_obj_t *footer = lv_label_create(scr);
    lv_label_set_text(footer, "Restart device to recover");
    lv_obj_set_style_text_color(footer, lv_color_hex(0x888888), 0);
    lv_obj_set_style_text_font(footer, &lv_font_montserrat_14, 0);
    lv_obj_align(footer, LV_ALIGN_BOTTOM_MID, 0, -10);

    /* Spin forever, keeping the screen visible */
    while (1) {
        lv_timer_handler();
        lv_tick_inc(5);
    }
}

void fatal_handler(const char *file, int line, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    error_screen(file, line, fmt, args);
    va_end(args);
}
