#!/usr/bin/env bash
# -- Build script for Linux prototype ---------------------------------------
# Usage:
#   ./scripts/build.sh          Debug build
#   ./scripts/build.sh release  Release build
#   ./scripts/build.sh clean    Clean build directory
set -euo pipefail

PROJECT_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="${PROJECT_ROOT}/build"

# -- Check for dependencies ---------------------------------------------
check_dep() {
    if ! command -v "$1" &>/dev/null; then
        echo "❌ Missing: $1 - install it and try again." >&2
        exit 1
    fi
}

check_dep cmake
check_dep pkg-config
check_dep autoconf
check_dep automake
check_dep libtool

if ! pkg-config --exists sdl2; then
    echo "❌ SDL2 not found. Install:  sudo apt install libsdl2-dev" >&2
    exit 1
fi

# -- Clone submodules if missing ---------------------------------------
clone_if_missing() {
    local dir="$1"
    local repo="$2"
    local branch="${3:-}"
    if [ ! -d "${PROJECT_ROOT}/${dir}" ]; then
        echo "Cloning ${repo} -> ${dir} …"
        if [ -n "$branch" ]; then
            git -C "$PROJECT_ROOT" clone --depth 1 --branch "$branch" "$repo" "$dir"
        else
            git -C "$PROJECT_ROOT" clone --depth 1 "$repo" "$dir"
        fi
    fi
}

clone_if_missing "external/lvgl"           "https://github.com/lvgl/lvgl.git"                  "v9.2.0"

clone_if_missing "external/libwally-core"   "https://github.com/ElementsProject/libwally-core.git" "release_1.5.6"

# libwally-core has its own submodules (secp256k1) - init them
if [ -d "${PROJECT_ROOT}/external/libwally-core" ]; then
    git -C "${PROJECT_ROOT}/external/libwally-core" submodule update --init --recursive
fi

# -- Clean -------------------------------------------------------------
if [ "${1:-}" = "clean" ]; then
    echo "Cleaning build directory…"
    rm -rf "$BUILD_DIR"
    echo "✅ Clean."
    exit 0
fi

# -- Configure & Build ------------------------------------------------
BUILD_TYPE="${1:-Debug}"
BUILD_TYPE="${BUILD_TYPE^}"   # Capitalize first letter

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

echo "Configuring (${BUILD_TYPE})…"
cmake "$PROJECT_ROOT" \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DLVGL_DIR="${PROJECT_ROOT}/external/lvgl"

echo "Building…"
cmake --build . -- -j"$(nproc)"

echo ""
echo "✅ Build complete: ${BUILD_DIR}/esp32_entropy"
echo "   Run it:  ${BUILD_DIR}/esp32_entropy"
