# -- ESP32 Platform Build ---------------------------------------------------
# This file is included when building with ESP-IDF (idf.py build)

# ESP-IDF provides its own LVGL component and build system.
# This file serves as documentation / placeholder for ESP-specific config.

# When using ESP-IDF, add these components via idf.py menuconfig or
# CMakeLists.txt in the main component:

# Required ESP-IDF components:
#   - lvgl            (LVGL graphics library)
#   - lvgl_touch      (touch input driver, e.g. FT5x06, GT911)
#   - lvgl_tft        (display driver, e.g. ILI9341, ST7789)

# libwally can be built as an ESP-IDF component in components/libwally/
# See platform/esp32/README.md for setup instructions.

message(STATUS "ESP32 platform selected - use idf.py build")
