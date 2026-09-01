# seedmix - Combine multiple BIP39 mnemonics statelessly

**LVGL-based touchscreen application**  
Prototype on Linux (SDL2) -> deploy to ESP32.

## Quick Start (Linux)

### 1. Install system dependencies
```bash
sudo apt install build-essential cmake libsdl2-dev autoconf automake libtool libtool-bin
```

### 2. Clone and build
```bash
# Clone the project with submodules (lvgl, libwally-core)
git clone --recurse-submodules <this-repo>
cd seedmix

# Build
chmod +x scripts/build.sh
./scripts/build.sh
```

### 3. Run
```bash
./build_linux/seedmix
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
