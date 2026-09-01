/**
 * @file buttons.c
 * @brief Physical GPIO buttons - raw hardware layer.
 *
 * Reports raw button state only
 */

#include "buttons.h"

#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"
#include "sdkconfig.h"

#if CONFIG_SEEDMIX_BUTTONS_ENABLE

static const char* TAG = "buttons";

static const int s_gpios[] = {
#if CONFIG_SEEDMIX_BUTTONS_COUNT > 0
    CONFIG_SEEDMIX_BUTTON_0_GPIO,
#endif
#if CONFIG_SEEDMIX_BUTTONS_COUNT > 1
    CONFIG_SEEDMIX_BUTTON_1_GPIO,
#endif
#if CONFIG_SEEDMIX_BUTTONS_COUNT > 2
    CONFIG_SEEDMIX_BUTTON_2_GPIO,
#endif
#if CONFIG_SEEDMIX_BUTTONS_COUNT > 3
    CONFIG_SEEDMIX_BUTTON_3_GPIO,
#endif
};

#endif /* CONFIG_SEEDMIX_BUTTONS_ENABLE */

void buttons_init(void) {
#if CONFIG_SEEDMIX_BUTTONS_ENABLE
    for (int i = 0; i < CONFIG_SEEDMIX_BUTTONS_COUNT; i++) {
        if (s_gpios[i] < 0) {
            continue;
        }
        /* Input-only pads (ESP32 GPIO 34-39) have no internal pull resistors.
         * The T-Display buttons use board-mounted external pull-ups, so for
         * those pins we simply leave the internal pull disabled. */
        const bool can_pull = GPIO_IS_VALID_OUTPUT_GPIO(s_gpios[i]);

        gpio_config_t cfg = {
            .pin_bit_mask = 1ULL << s_gpios[i],
            .mode         = GPIO_MODE_INPUT,
            .pull_up_en   = (CONFIG_SEEDMIX_BUTTONS_ACTIVE_LOW && can_pull) ? GPIO_PULLUP_ENABLE
                                                                            : GPIO_PULLUP_DISABLE,
            .pull_down_en = (!CONFIG_SEEDMIX_BUTTONS_ACTIVE_LOW && can_pull)
                                ? GPIO_PULLDOWN_ENABLE
                                : GPIO_PULLDOWN_DISABLE,
            .intr_type    = GPIO_INTR_DISABLE,
        };
        ESP_ERROR_CHECK(gpio_config(&cfg));
        ESP_LOGI(TAG, "button %d on GPIO %d (%s)", i, s_gpios[i],
                 CONFIG_SEEDMIX_BUTTONS_ACTIVE_LOW ? "active-low" : "active-high");
    }
#else
    (void)0;
#endif
}

int buttons_count(void) {
#if CONFIG_SEEDMIX_BUTTONS_ENABLE
    return CONFIG_SEEDMIX_BUTTONS_COUNT;
#else
    return 0;
#endif
}

bool button_is_pressed(int idx) {
#if CONFIG_SEEDMIX_BUTTONS_ENABLE
    if (idx < 0 || idx >= CONFIG_SEEDMIX_BUTTONS_COUNT || s_gpios[idx] < 0) {
        return false;
    }
    int level = gpio_get_level(s_gpios[idx]);
    return CONFIG_SEEDMIX_BUTTONS_ACTIVE_LOW ? (level == 0) : (level == 1);
#else
    (void)idx;
    return false;
#endif
}

bool buttons_any_pressed(void) {
    for (int i = 0; i < buttons_count(); i++) {
        if (button_is_pressed(i)) {
            return true;
        }
    }
    return false;
}
