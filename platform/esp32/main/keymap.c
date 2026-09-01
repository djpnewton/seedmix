/**
 * @file keymap.c
 * @brief Translation layer: physical buttons -> logical LVGL keys.
 *
 * The hardware layer (buttons.c) only reports raw button state.  This layer
 * debounces it and maps combinations to logical keys:
 *
 *   button 0 alone       -> SEEDMIX_KEY_1 ('1')
 *   button 1 alone       -> SEEDMIX_KEY_2 ('2')
 *   button 0 + button 1  -> LV_KEY_ENTER       (both held = confirm)
 */

#include "keymap.h"

#include "buttons.h"
#include "lvgl.h"
#include "sdkconfig.h"

#if CONFIG_SEEDMIX_BUTTONS_ENABLE

static lv_indev_t* s_indev    = NULL;
static lv_key_t    s_last_key = 0;

typedef struct {
    uint32_t last_raw;
    uint32_t stable_since;
    bool     state;
} debounce_t;

static debounce_t s_dbnc[CONFIG_SEEDMIX_BUTTONS_COUNT];

static bool debounced_pressed(int idx) {
    if (idx < 0 || idx >= CONFIG_SEEDMIX_BUTTONS_COUNT) {
        return false;
    }
    debounce_t* d   = &s_dbnc[idx];
    bool        raw = button_is_pressed(idx);
    uint32_t    now = lv_tick_get();

    if ((uint32_t)raw != d->last_raw) {
        d->last_raw     = (uint32_t)raw;
        d->stable_since = now;
    }
    if ((now - d->stable_since) >= (uint32_t)CONFIG_SEEDMIX_BUTTONS_DEBOUNCE_MS) {
        d->state = raw;
    }
    return d->state;
}

/* Map the current button combination to a single logical key (0 = none). */
static lv_key_t translate(void) {
    bool b0 = debounced_pressed(0);
    bool b1 = debounced_pressed(1);

    if (b0 && b1) {
        return LV_KEY_ENTER;
    }
    if (b0) {
        return SEEDMIX_KEY_1;
    }
    if (b1) {
        return SEEDMIX_KEY_2;
    }
    return 0;
}

static void keymap_read_cb(lv_indev_t* indev, lv_indev_data_t* data) {
    (void)indev;

    lv_key_t cur = translate();
    if (cur != 0) {
        s_last_key  = cur;
        data->state = LV_INDEV_STATE_PRESSED;
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }
    data->key = s_last_key;
}

#endif /* CONFIG_SEEDMIX_BUTTONS_ENABLE */

void keymap_init(void) {
#if CONFIG_SEEDMIX_BUTTONS_ENABLE
    s_indev = lv_indev_create();
    lv_indev_set_type(s_indev, LV_INDEV_TYPE_KEYPAD);
    lv_indev_set_read_cb(s_indev, keymap_read_cb);
#endif
}

lv_indev_t* keymap_get_indev(void) {
#if CONFIG_SEEDMIX_BUTTONS_ENABLE
    return s_indev;
#else
    return NULL;
#endif
}

lv_key_t keymap_current_key(void) {
#if CONFIG_SEEDMIX_BUTTONS_ENABLE
    return translate();
#else
    return 0;
#endif
}
