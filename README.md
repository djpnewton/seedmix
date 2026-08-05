# ESP32 Entropy - Combine multiple BIP39 mnemonics statelessly

**LVGL-based touchscreen application**  
Prototype on Linux (SDL2) -> deploy to ESP32.

```
esp32_entropy/
├-- CMakeLists.txt              # Top-level; detects platform
├-- main/
│   ├-- app.h                   # Shared app interface
│   ├-- main.c                  # App initialization & callbacks
│   ├-- lv_conf.h               # LVGL config (shared)
│   ├-- util/
│   │   ├-- error.h             # Fatal error macros + LVGL error screen
│   │   └-- log.h               # Logging macros
│   ├-- ui/
│   │   ├-- ui.h                # UI API
│   │   └-- ui.c                # Screen layouts
│   └-- crypto/
│       ├-- mnemonic.h / mnemonic.c  # BIP39 via libwally
│       └-- secure_stack.h / .c      # Guarded memory stack
├-- platform/
│   ├-- linux/
│   │   ├-- linux.cmake         # Linux CMake toolchain
│   │   └-- main_linux.c        # SDL2 entry point
│   └-- esp32/
│       ├-- esp32.cmake         # ESP32 CMake stub
│       ├-- main_esp32.c        # ESP-IDF entry point
│       └-- README.md           # ESP32 setup instructions
├-- cmake/
│   └-- BuildLibWally.cmake     # Builds libwally from submodule
├-- scripts/
│   └-- build.sh                # One-shot Linux build
├-- .gitignore
├-- .gitmodules
└-- README.md
```

## Quick Start (Linux)

### 1. Install system dependencies
```bash
sudo apt install build-essential cmake libsdl2-dev autoconf automake libtool
```

### 2. Clone and build
```bash
# Clone the project with submodules (lvgl, libwally-core)
git clone --recurse-submodules <this-repo>
cd esp32_entropy

# Build
chmod +x scripts/build.sh
./scripts/build.sh
```

### 3. Run
```bash
./build/esp32_entropy
```

A 480×320 window opens with a "Generate" button that creates a BIP39 mnemonic.

## ESP32 Deployment

See [platform/esp32/README.md](platform/esp32/README.md) for:
- Required ESP-IDF components
- Touchscreen wiring
- libwally cross-compilation

## Architecture

| Layer          | Description                                      |
|----------------|--------------------------------------------------|
| `main/ui/`     | LVGL screens, platform-agnostic                  |
| `main/crypto/` | Wallet operations, wraps libwally                |
| `platform/`    | Platform entry points (SDL2 / FreeRTOS)          |
| `cmake/`       | Reusable CMake modules                           |

Platform code is only the entry point, tick provider, and display/input driver
setup.  All business logic and UI lives in `main/`.
