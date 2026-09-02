cmake_minimum_required(VERSION 3.16)

if(ESP_PLATFORM)
    message(FATAL_ERROR
        "This is the Linux prototype build.\n"
        "The ESP32 firmware is a separate project - build it with:\n"
        "    cd platform/esp32 && idf.py build")
endif()

project(seedmix VERSION 0.1.0 LANGUAGES C)

option(ENABLE_ASAN "Enable AddressSanitizer" OFF)
option(BUILD_TESTS "Build unit tests (Linux only)" OFF)
option(ENABLE_COVERAGE "Enable code coverage instrumentation (gcov/lcov)" OFF)
set(ENABLE_SANITIZERS "" CACHE STRING "Comma-separated sanitizers for tests (e.g. address,undefined)")

# -- Platform Detection -----------------------------------------------------
# This top-level CMakeLists.txt builds the Linux/SDL2 prototype only.
# The ESP32 firmware is a separate ESP-IDF project built with idf.py
if(CMAKE_SYSTEM_NAME STREQUAL "Generic")
    message(FATAL_ERROR "ESP32 builds use ESP-IDF")
endif()
set(PLATFORM "linux")
message(STATUS "Building for Linux (prototype)")

# -- Common Source Files ----------------------------------------------------
set(COMMON_SOURCES
    main/main.c
    main/ui/ui.c
    main/assets/splash_480x320_img.c
    main/assets/splash_240x135_img.c
    main/assets/logo_img.c
    main/ui/word_entry.c
    main/ui/mnemonic_view.c
    main/ui/log.c
    main/crypto/mnemonic.c
    main/crypto/touch.c
    main/crypto/dice.c
    main/crypto/coin.c
    main/crypto/secure_stack.c
    main/crypto/bip39_wordlist.c
    main/crypto/seedqr.c
    main/qr/qr.c
    main/util/error.c
    main/util/log.c
    main/util/utils.c
)

# -- Platform-Specific Setup ------------------------------------------------
include(${CMAKE_SOURCE_DIR}/platform/linux/linux.cmake)

# -- Tests -------------------------------------------------------------------
if(BUILD_TESTS)
    if(PLATFORM STREQUAL "linux")
        enable_testing()
        add_subdirectory(tests)
    else()
        message(WARNING "Tests are only supported on the Linux platform")
    endif()
endif()
