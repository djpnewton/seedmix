/* SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file platform/esp32/main_esp32.c
 * @brief ESP-IDF entry point.
 *
 * Initializes LVGL with the configured TFT + touch drivers and then
 * hands control to the shared application logic.
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "lvgl.h"
#include "app.h"

static const char *TAG = "main";

/* -- LVGL tick for FreeRTOS ------------------------------------------- */
uint32_t lv_tick_get(void) {
    return xTaskGetTickCount() * portTICK_PERIOD_MS;
}

/* -- Application task ------------------------------------------------- */
static void app_task(void *pvParameter) {
    (void)pvParameter;

    /* LVGL and display/touch drivers are initialized by ESP-IDF components
       via Kconfig - see platform/esp32/README.md */

    app_init();

    while (1) {
        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}

/* -- Entry point ------------------------------------------------------ */
void app_main(void) {
    ESP_LOGI(TAG, "Starting Entropy…");
    xTaskCreate(app_task, "lvgl_task", 4096, NULL, 5, NULL);
}
