#!/usr/bin/env bash
# -- Build / flash / monitor the ESP32 firmware -----------------------------
# Usage:
#   ./scripts/build_esp32.sh              Build (into build_ttgo_tdisplay/)
#   ./scripts/build_esp32.sh flash        Build + flash via idf.py
#   ./scripts/build_esp32.sh monitor      Open the serial monitor
#   ./scripts/build_esp32.sh menuconfig   Edit the Kconfig (seedmix Hardware)
#   ./scripts/build_esp32.sh clean        Remove the build dir
#   ./scripts/build_esp32.sh fullclean    Also delete sdkconfig
#
# Environment overrides:
#   IDF_ACTIVATE  Path to the ESP-IDF activate script
#                 (default: ~/.espressif/tools/activate_idf_v5.5.5.sh)
#   ESP_PORT      Serial port for flash/monitor (default: /dev/ttyUSB0)
set -euo pipefail

PROJECT_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
ESP32_DIR="${PROJECT_ROOT}/platform/esp32"
BUILD_DIR="${PROJECT_ROOT}/build_ttgo_tdisplay"
IDF_ACTIVATE="${IDF_ACTIVATE:-$HOME/.espressif/tools/activate_idf_v5.5.5.sh}"
ESP_PORT="${ESP_PORT:-/dev/ttyACM0}"

ACTION="${1:-build}"

if [ ! -f "$IDF_ACTIVATE" ]; then
    echo "ESP-IDF activate script not found: $IDF_ACTIVATE" >&2
    echo "Set IDF_ACTIVATE=/path/to/activate_idf_vX.Y.Z.sh" >&2
    exit 1
fi

# idf.py needs the ESP-IDF environment (toolchain, Python venv, IDF_PATH).
# The activate script only works when it detects it's being sourced by a
# shell, so delegate to `bash -c` (where $0 is "bash") and pass state through
# the environment.  Inside we call idf.py by path because the activate script
# exposes it as a shell alias, which is not expanded in non-interactive shells.
IDF_ACTIVATE="$IDF_ACTIVATE" BUILD_DIR="$BUILD_DIR" ESP_PORT="$ESP_PORT" \
    ESP32_DIR="$ESP32_DIR" ACTION="$ACTION" \
bash -c '
    set -e
    # shellcheck disable=SC1090
    source "$IDF_ACTIVATE" >/dev/null

    cd "$ESP32_DIR"

    if [ "$ACTION" = "clean" ] || [ "$ACTION" = "fullclean" ]; then
        python "$IDF_PATH/tools/idf.py" -B "$BUILD_DIR" "$ACTION"
        exit 0
    fi

    # Configure the target once; sdkconfig lives in the project directory.
    # Regenerate it when sdkconfig.defaults is newer so changed defaults apply
    # (idf.py only reads the defaults file when sdkconfig is first created).
    if [ ! -f sdkconfig ] || [ sdkconfig.defaults -nt sdkconfig ]; then
        rm -f sdkconfig
        python "$IDF_PATH/tools/idf.py" -B "$BUILD_DIR" set-target esp32
    fi

    case "$ACTION" in
        flash)   python "$IDF_PATH/tools/idf.py" -B "$BUILD_DIR" -p "$ESP_PORT" flash ;;
        monitor) python "$IDF_PATH/tools/idf.py" -B "$BUILD_DIR" -p "$ESP_PORT" monitor ;;
        *)       python "$IDF_PATH/tools/idf.py" -B "$BUILD_DIR" "$ACTION" ;;
    esac
'
