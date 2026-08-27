cmake_minimum_required(VERSION 3.16)
project(seedmix VERSION 0.1.0 LANGUAGES C)

option(ENABLE_ASAN "Enable AddressSanitizer" OFF)

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
    main/ui/word_entry.c
    main/ui/mnemonic_view.c
    main/ui/log.c
    main/crypto/mnemonic.c
    main/crypto/secure_stack.c
    main/crypto/bip39_wordlist.c
    main/util/error.c
    main/util/log.c
    main/util/utils.c
)

# -- Platform-Specific Setup ------------------------------------------------
if(PLATFORM STREQUAL "linux")
    include(${CMAKE_SOURCE_DIR}/platform/linux/linux.cmake)
elseif(PLATFORM STREQUAL "esp32")
    include(${CMAKE_SOURCE_DIR}/platform/esp32/esp32.cmake)
endif()
