/**
 * @file debug_screen.c
 * @brief Hardware debug screen: config summary + live button test.
 *
 * Pressing a physical button switches to a full-screen "pressed" indicator
 * for as long as the button is held, then returns to the summary screen.
 */

#include "debug_screen.h"

#include "buttons.h"
#include "display.h"
#include "graphics_test.h"
#include "keymap.h"
#include "lvgl.h"
#include "sdkconfig.h"

#include <stdio.h>

static lv_obj_t* s_debug_scr      = NULL;
static lv_obj_t* s_pressed_scr    = NULL;
static lv_obj_t* s_pressed_label  = NULL;
static lv_obj_t* s_gfx_scr        = NULL;
static bool      s_pressed_active = false;
static bool      s_gfx_active     = false;
static uint32_t  s_last_enter_ms  = 0;
static lv_key_t  s_prev_key       = 0;

static const char* lv_key_name(uint32_t key) {
    switch (key) {
    case LV_KEY_ENTER:
        return "ENTER";
    case LV_KEY_NEXT:
        return "NEXT";
    case LV_KEY_PREV:
        return "PREV";
    case LV_KEY_ESC:
        return "ESC";
    case LV_KEY_UP:
        return "UP";
    case LV_KEY_DOWN:
        return "DOWN";
    case LV_KEY_LEFT:
        return "LEFT";
    case LV_KEY_RIGHT:
        return "RIGHT";
    case LV_KEY_HOME:
        return "HOME";
    case LV_KEY_END:
        return "END";
    default:
        return "?";
    }
}

static void show_graphics_test(void) {
    s_gfx_active = true;
    lv_screen_load(s_gfx_scr);
}

/* Switch screens based on the current logical key (debounced). */
static void debug_poll_cb(lv_timer_t* t) {
    (void)t;

    lv_key_t key = keymap_current_key();

    if (s_gfx_active) {
        /* Graphics test screen: ENTER returns to the summary screen. */
        if (key == LV_KEY_ENTER && s_prev_key != LV_KEY_ENTER) {
            s_gfx_active    = false;
            s_last_enter_ms = 0;
            lv_screen_load(s_debug_scr);
        }
        s_prev_key = key;
        return;
    }

    /* ENTER twice in quick succession activates the graphics test. */
    if (key == LV_KEY_ENTER && s_prev_key != LV_KEY_ENTER) {
        uint32_t now = lv_tick_get();
        if (now - s_last_enter_ms <= 800) {
            s_last_enter_ms = 0;
            s_prev_key      = key;
            show_graphics_test();
            return;
        }
        s_last_enter_ms = now;
    }
    s_prev_key = key;

    if (key != 0) {
        if (!s_pressed_active) {
            s_pressed_active = true;
            lv_screen_load(s_pressed_scr);
        }
        // Keep the label live so it reflects the current combination
        lv_label_set_text_fmt(s_pressed_label, "PRESSED: %s", lv_key_name(key));
    } else if (s_pressed_active) {
        s_pressed_active = false;
        lv_screen_load(s_debug_scr);
    }
}

void debug_screen_init(void) {
    const char* touch   = "none";
    const char* buttons = "none";
    const char* camera  = "none";
    const char* trng    = "none";
#if CONFIG_SEEDMIX_TOUCHSCREEN_ENABLE
    touch = "enabled";
#endif
#if CONFIG_SEEDMIX_BUTTONS_ENABLE
    char buttons_buf[16];
    snprintf(buttons_buf, sizeof(buttons_buf), "enabled (%d)", CONFIG_SEEDMIX_BUTTONS_COUNT);
    buttons = buttons_buf;
#endif
#if CONFIG_SEEDMIX_CAMERA_ENABLE
    camera = "enabled";
#endif
#if CONFIG_SEEDMIX_TRNG_SOURCE_HARDWARE
    trng = "HW RNG";
#endif

    /* -- Debug (summary) screen -------------------------------------- */
    s_debug_scr = lv_obj_create(NULL);

    lv_obj_t* info = lv_label_create(s_debug_scr);
    lv_obj_set_style_text_font(info, &lv_font_montserrat_14, 0);
#if CONFIG_SEEDMIX_DISPLAY_DRIVER_NONE
    lv_label_set_text(info, "seedmix ESP32\nheadless (no display)");
#else
    lv_label_set_text_fmt(info,
                          "seedmix ESP32\n"
                          "display %dx%d\n"
                          "touch %s\n"
                          "camera %s\n"
                          "buttons %s\n"
                          "trng %s",
                          DISPLAY_WIDTH, DISPLAY_HEIGHT, touch, camera, buttons, trng);
#endif
    lv_obj_align(info, LV_ALIGN_TOP_LEFT, 4, 4);

    /* -- Pressed (hold indicator) screen ------------------------------ */
    s_pressed_scr   = lv_obj_create(NULL);
    s_pressed_label = lv_label_create(s_pressed_scr);
    lv_obj_set_style_text_font(s_pressed_label, &lv_font_montserrat_14, 0);
    lv_label_set_text(s_pressed_label, "PRESSED");
    lv_obj_center(s_pressed_label);

    /* -- Graphics test screen ---------------------------------------- */
    s_gfx_scr = graphics_test_create();

    lv_screen_load(s_debug_scr);

    /* Poll the raw button state to switch screens on press/release. */
    lv_timer_create(debug_poll_cb, 20, NULL);
}
