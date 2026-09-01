/**
 * @file main_esp32.c
 * @brief ESP-IDF entry point for seedmix.
 *
 * Brings up LVGL with the configured display, touchscreen and physical
 * buttons, then runs the LVGL loop.  This is currently a platform bring-up:
 * the shared app sources (main/main.c + ui + crypto) are wired in once
 * libwally/qrencode/quirc are available as IDF components.
 */

#include "buttons.h"
#include "debug_screen.h"
#include "display.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "keymap.h"
#include "lvgl.h"
#include "sdkconfig.h"

static const char* TAG = "seedmix";

/* -- LVGL tick source (milliseconds) ---------------------------------- */
static uint32_t lvgl_tick_cb(void) { return (uint32_t)(esp_timer_get_time() / 1000); }

/* -- Entry point ------------------------------------------------------- */
void app_main(void) {
    ESP_LOGI(TAG, "seedmix ESP32 debug screen");

    lv_init();
    lv_tick_set_cb(lvgl_tick_cb);

    display_init();
    buttons_init();
    keymap_init();

    // TODO: call app_init() once the shared app sources are compiled in
    debug_screen_init();

    while (1) {
        lv_timer_handler();
        /* At least one FreeRTOS tick so the idle tasks get to run.  With the
         * default 100 Hz tick rate vTaskDelay(pdMS_TO_TICKS(5)) rounds down to
         * 0 ticks, which busy-spins and trips the task watchdog. */
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
