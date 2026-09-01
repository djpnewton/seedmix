# seedmix main component: platform bring-up (display, buttons, HAL) and the
# application entry point (main_esp32.c).
#
# The shared app sources under ../../../main are not compiled here yet - they
# depend on libwally / qrencode / quirc which must first be built as ESP-IDF
# components (see platform/esp32/README.md).

set(SHARED_MAIN_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../main")

idf_component_register(
    SRCS
        "main_esp32.c"
        "debug_screen.c"
        "graphics_test.c"
        "keymap.c"
        "display.c"
        "buttons.c"
        "hal_esp32.c"
    INCLUDE_DIRS
        "."
        "${SHARED_MAIN_DIR}"
        "${SHARED_MAIN_DIR}/util"
        "${SHARED_MAIN_DIR}/crypto"
        "${SHARED_MAIN_DIR}/qr"
        "${SHARED_MAIN_DIR}/ui"
    REQUIRES
        lvgl
        esp_lcd
        driver
        esp_timer
        esp_hw_support
)
