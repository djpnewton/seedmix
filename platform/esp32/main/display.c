/**
 * @file display.c
 * @brief ESP-IDF LVGL display driver using esp_lcd over SPI
 */

#include "display.h"

#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_heap_caps.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_st7789.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_log.h"
#include "lvgl.h"

static const char* TAG = "display";

/* Render buffer: a horizontal strip of the screen (partial refresh). */
#define DRAW_BUF_LINES 40
#define DRAW_BUF_SIZE (DISPLAY_WIDTH * DRAW_BUF_LINES * sizeof(uint16_t))

static esp_lcd_panel_handle_t s_panel       = NULL;
static lv_display_t*          s_display     = NULL;
static uint8_t*               s_draw_buf[2] = {NULL, NULL};

/* -- LVGL flush callback ---------------------------------------------- */
static void lvgl_flush_cb(lv_display_t* disp, const lv_area_t* area, uint8_t* px_map) {
    esp_lcd_panel_draw_bitmap(s_panel, area->x1, area->y1, area->x2 + 1, area->y2 + 1,
                              (const void*)px_map);
    lv_display_flush_ready(disp);
}

/* -- Init -------------------------------------------------------------- */
void display_init(void) {
#if CONFIG_SEEDMIX_DISPLAY_DRIVER_NONE
    ESP_LOGW(TAG, "display driver is 'None'; no display initialized");
    return;
#endif

#if defined(CONFIG_SEEDMIX_DISPLAY_DRIVER_ILI9341) || defined(CONFIG_SEEDMIX_DISPLAY_DRIVER_ST7735)
#error                                                                                             \
    "ILI9341/ST7735 are not in-tree. Add espressif/esp_lcd_ili9341 or espressif/esp_lcd_st7735 to idf_component.yml and extend this file."
#endif

    ESP_LOGI(TAG, "init %dx%d ST7789", DISPLAY_WIDTH, DISPLAY_HEIGHT);

    // Backlight (if wired)
    if (CONFIG_SEEDMIX_DISPLAY_BACKLIGHT_GPIO >= 0) {
        gpio_config_t bl = {
            .pin_bit_mask = 1ULL << CONFIG_SEEDMIX_DISPLAY_BACKLIGHT_GPIO,
            .mode         = GPIO_MODE_OUTPUT,
            .intr_type    = GPIO_INTR_DISABLE,
        };
        gpio_config(&bl);
        gpio_set_level(CONFIG_SEEDMIX_DISPLAY_BACKLIGHT_GPIO, 1);
    }

    // SPI bus
    spi_bus_config_t bus_cfg = {
        .sclk_io_num     = CONFIG_SEEDMIX_DISPLAY_SPI_CLK_GPIO,
        .mosi_io_num     = CONFIG_SEEDMIX_DISPLAY_SPI_MOSI_GPIO,
        .miso_io_num     = -1,
        .quadwp_io_num   = -1,
        .quadhd_io_num   = -1,
        .max_transfer_sz = DISPLAY_WIDTH * DISPLAY_HEIGHT * sizeof(uint16_t) + 16,
    };
    ESP_ERROR_CHECK(spi_bus_initialize(CONFIG_SEEDMIX_DISPLAY_SPI_HOST, &bus_cfg, SPI_DMA_CH_AUTO));

    // Panel IO (SPI)
    esp_lcd_panel_io_handle_t     io     = NULL;
    esp_lcd_panel_io_spi_config_t io_cfg = {
        .cs_gpio_num       = CONFIG_SEEDMIX_DISPLAY_SPI_CS_GPIO,
        .dc_gpio_num       = CONFIG_SEEDMIX_DISPLAY_SPI_DC_GPIO,
        .spi_mode          = 0,
        .pclk_hz           = 20 * 1000 * 1000,
        .trans_queue_depth = 10,
        .lcd_cmd_bits      = 8,
        .lcd_param_bits    = 8,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi(
        (esp_lcd_spi_bus_handle_t)CONFIG_SEEDMIX_DISPLAY_SPI_HOST, &io_cfg, &io));

    // Panel
    esp_lcd_panel_dev_config_t panel_cfg = {
        .reset_gpio_num = CONFIG_SEEDMIX_DISPLAY_RST_GPIO,
        .rgb_ele_order  = LCD_RGB_ELEMENT_ORDER_RGB,
        .bits_per_pixel = 16,
        .data_endian    = LCD_RGB_DATA_ENDIAN_LITTLE,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(io, &panel_cfg, &s_panel));

    ESP_ERROR_CHECK(esp_lcd_panel_reset(s_panel));
    ESP_ERROR_CHECK(esp_lcd_panel_init(s_panel));

    // Orientation / appearance (Kconfig bools may be undefined when off)
    bool invert   = false;
    bool swap_xy  = false;
    bool mirror_x = false;
    bool mirror_y = false;
#if CONFIG_SEEDMIX_DISPLAY_INVERT_COLORS
    invert = true;
#endif
#if CONFIG_SEEDMIX_DISPLAY_SWAP_XY
    swap_xy = true;
#endif
#if CONFIG_SEEDMIX_DISPLAY_MIRROR_X
    mirror_x = true;
#endif
#if CONFIG_SEEDMIX_DISPLAY_MIRROR_Y
    mirror_y = true;
#endif
    esp_lcd_panel_invert_color(s_panel, invert);
    esp_lcd_panel_swap_xy(s_panel, swap_xy);
    esp_lcd_panel_mirror(s_panel, mirror_x, mirror_y);
    esp_lcd_panel_set_gap(s_panel, CONFIG_SEEDMIX_DISPLAY_OFFSET_X,
                          CONFIG_SEEDMIX_DISPLAY_OFFSET_Y);
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(s_panel, true));

    // LVGL display + double partial-render buffers.  Two buffers are
    // required: esp_lcd_panel_draw_bitmap() is asynchronous (DMA), so while
    // one strip is being sent the other must be free for LVGL to render
    // into - otherwise the next strip overwrites the one still in flight
    s_display = lv_display_create(DISPLAY_WIDTH, DISPLAY_HEIGHT);
    lv_display_set_color_format(s_display, LV_COLOR_FORMAT_RGB565);

    for (int i = 0; i < 2; i++) {
        s_draw_buf[i] = heap_caps_malloc(DRAW_BUF_SIZE, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
        if (!s_draw_buf[i]) {
            s_draw_buf[i] = heap_caps_malloc(DRAW_BUF_SIZE, MALLOC_CAP_INTERNAL);
        }
        ESP_ERROR_CHECK(s_draw_buf[i] ? ESP_OK : ESP_ERR_NO_MEM);
    }

    lv_display_set_buffers(s_display, s_draw_buf[0], s_draw_buf[1], DRAW_BUF_SIZE,
                           LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_flush_cb(s_display, lvgl_flush_cb);
}
