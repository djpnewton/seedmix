cmake_minimum_required(VERSION 3.16)
project(esp32_entropy VERSION 0.1.0 LANGUAGES C)

# -- Platform Detection -----------------------------------------------------
# ESP-IDF sets CMAKE_SYSTEM_NAME when building; otherwise we assume Linux
if(CMAKE_SYSTEM_NAME STREQUAL "Generic")
    set(PLATFORM "esp32")
    message(STATUS "Building for ESP32")
else()
    set(PLATFORM "linux")
    message(STATUS "Building for Linux (prototype)")
endif()

# -- Common Source Files ----------------------------------------------------
set(COMMON_SOURCES
    main/main.c
    main/ui/ui.c
    main/crypto/mnemonic.c
    main/crypto/secure_stack.c
    main/util/error.c
)

# -- Platform-Specific Setup ------------------------------------------------
if(PLATFORM STREQUAL "linux")
    include(${CMAKE_SOURCE_DIR}/platform/linux/linux.cmake)
elseif(PLATFORM STREQUAL "esp32")
    include(${CMAKE_SOURCE_DIR}/platform/esp32/esp32.cmake)
endif()
