#!/usr/bin/env bash
# -- Generate code coverage report (Linux only) -----------------------------
# Usage:
#   ./scripts/coverage.sh
# Builds with coverage instrumentation, runs the tests, and writes an HTML
# report to build_linux/coverage/index.html (plus a text summary to stdout).
set -euo pipefail

PROJECT_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="${PROJECT_ROOT}/build_linux"

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
check_dep lcov
check_dep genhtml

# SDL2 is required to configure the top-level project (linux.cmake)
if ! pkg-config --exists sdl2; then
    echo "SDL2 not found. Install:  sudo apt install libsdl2-dev" >&2
    exit 1
fi

# Clone dependencies if missing
source "${PROJECT_ROOT}/scripts/ensure_deps.sh"

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

echo "Configuring (with coverage)…"
cmake "$PROJECT_ROOT" \
    -DCMAKE_BUILD_TYPE=Debug \
    -DBUILD_TESTS=ON \
    -DENABLE_COVERAGE=ON \
    -DLVGL_DIR="${PROJECT_ROOT}/external/lvgl"

echo "Building tests…"
cmake --build . --target seedmix_tests -- -j"$(nproc)"

echo "Running tests…"
ctest --output-on-failure

echo "Collecting coverage…"
lcov --capture --directory . --output-file coverage.info

# Keep only project sources: drop system headers, dependencies, and test stubs.
# --ignore-errors unused: some patterns (e.g. /usr/*) legitimately match nothing.
lcov --ignore-errors unused --remove coverage.info \
    '/usr/*' \
    "${PROJECT_ROOT}/external/*" \
    "${PROJECT_ROOT}/tests/*" \
    "${BUILD_DIR}/*" \
    --output-file coverage.info

# Print the summary and fail if line coverage drops below the threshold.
summary="$(lcov --summary coverage.info)"
printf '%s\n' "$summary"

line_pct="$(printf '%s\n' "$summary" | awk '/lines/{gsub(/%/, "", $2); print $2; exit}')"
if [ -z "$line_pct" ]; then
    echo "Could not parse coverage percentage" >&2
    exit 1
fi
if ! awk -v p="$line_pct" 'BEGIN { exit (p + 0 >= 95) ? 0 : 1 }'; then
    echo "Line coverage ${line_pct}% is below the 95% threshold" >&2
    exit 1
fi

mkdir -p coverage
genhtml coverage.info --output-directory coverage

echo ""
echo "Coverage report written to ${BUILD_DIR}/coverage/index.html"
