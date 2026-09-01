#!/usr/bin/env bash
# -- Ensure external dependencies are present ------------------------------
# Clones lvgl + libwally-core (and libwally's secp256k1 submodule) if missing.

set -euo pipefail

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

if ! command -v git &>/dev/null; then
    echo "Missing: git" >&2
    exit 1
fi

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

clone_if_missing "external/lvgl" "https://github.com/lvgl/lvgl.git" "v9.2.0"
clone_if_missing "external/libwally-core" "https://github.com/ElementsProject/libwally-core.git" "release_1.5.6"
clone_if_missing "external/libqrencode" "https://github.com/fukuchi/libqrencode.git" "v4.1.1"
clone_if_missing "external/quirc" "https://github.com/dlbeer/quirc.git" "v1.2"

# libwally-core has its own submodules (secp256k1) - init them
if [ -d "${PROJECT_ROOT}/external/libwally-core" ]; then
    git -C "${PROJECT_ROOT}/external/libwally-core" submodule update --init --recursive
fi
