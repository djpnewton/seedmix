#!/usr/bin/env bash
# -- Run unit tests (Linux only) -------------------------------------------
# Usage:
#   ./scripts/test.sh              Configure (if needed), build, and run tests
#   ./scripts/test.sh -R utils     Run only tests whose name matches "utils"
#   ./scripts/test.sh -V           Verbose CTest output
#
# Extra arguments are forwarded to ctest (e.g. -R, -V, --rerun-failed).
set -euo pipefail

PROJECT_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="${PROJECT_ROOT}/build"

# -- Check for dependencies ---------------------------------------------
check_dep() {
    if ! command -v "$1" &>/dev/null; then
        echo "Missing: $1 - install it and try again." >&2
        exit 1
    fi
}

check_dep cmake
check_dep pkg-config
check_dep autoconf
check_dep automake
check_dep libtool

# SDL2 is required to configure the top-level project (linux.cmake)
if ! pkg-config --exists sdl2; then
    echo "SDL2 not found. Install:  sudo apt install libsdl2-dev" >&2
    exit 1
fi

# Clone dependencies if missing
source "${PROJECT_ROOT}/scripts/ensure_deps.sh"

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

echo "Configuring (with tests)…"
cmake "$PROJECT_ROOT" \
    -DCMAKE_BUILD_TYPE=Debug \
    -DBUILD_TESTS=ON \
    -DLVGL_DIR="${PROJECT_ROOT}/external/lvgl"

echo "Building tests…"
cmake --build . --target seedmix_tests -- -j"$(nproc)"

echo "Running tests…"
ctest --output-on-failure "$@"
