#!/usr/bin/env bash
# -- Web (Emscripten) build script ------------------------------------------
# Usage:
#   ./scripts/build_web.sh          Configure + build (Release) into build_web/
#   ./scripts/build_web.sh serve    Build, then serve http://localhost:8000
#   ./scripts/build_web.sh clean    Remove build_web/
#
# The Emscripten SDK is auto-installed into external/emsdk on first run (see
# ensure_deps.sh).  Set EMSDK=/path/to/emsdk to use an existing SDK instead.
set -euo pipefail

PROJECT_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="${PROJECT_ROOT}/build_web"

check_dep() {
    if ! command -v "$1" &>/dev/null; then
        echo "❌ Missing: $1 - $2" >&2
        exit 1
    fi
}

# -- Clone pinned deps + install the Emscripten SDK ------------------------
# shellcheck disable=SC1091
source "${PROJECT_ROOT}/scripts/ensure_deps.sh"

if [ -n "${EMSDK:-}" ] && [ -f "${EMSDK}/emsdk_env.sh" ]; then
    EMSDK_DIR="$EMSDK"   # use a pre-existing SDK if one is provided
else
    ensure_emsdk
    EMSDK_DIR="${PROJECT_ROOT}/external/emsdk"
fi

# shellcheck disable=SC1091
source "${EMSDK_DIR}/emsdk_env.sh"

check_dep emcc "could not activate the Emscripten SDK"
check_dep emcmake "could not activate the Emscripten SDK"
check_dep emconfigure "could not activate the Emscripten SDK"
check_dep emmake "could not activate the Emscripten SDK"

if [ "${1:-}" = "clean" ]; then
    echo "Cleaning build directory…"
    rm -rf "$BUILD_DIR"
    echo "✅ Clean."
    exit 0
fi

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

echo "Configuring (Emscripten, Release)…"
emcmake cmake "$PROJECT_ROOT" \
    -DCMAKE_BUILD_TYPE=Release

echo "Building…"
cmake --build . -- -j"$(nproc)"

echo ""
echo "✅ Build complete: ${BUILD_DIR}/seedmix.html"
echo "   Serve it:  python3 -m http.server 8000 --directory ${BUILD_DIR}"
echo "   Then open: http://localhost:8000/seedmix.html"
echo "   (Camera access needs a secure context: http://localhost or https)"

if [ "${1:-}" = "serve" ]; then
    echo ""
    echo "Serving on http://localhost:8000/ (Ctrl-C to stop)…"
    python3 -m http.server 8000 --directory "$BUILD_DIR"
fi
