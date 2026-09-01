# ESP32 Build Instructions (TTGO T-Display)

The default configuration (`sdkconfig.defaults`) targets the **classic
TTGO T-Display**: ESP32, 240x135 ST7789 SPI display, no touchscreen, two
physical buttons (GPIO 0 top, GPIO 35 bottom), no camera, hardware RNG.

## Prerequisites

- ESP-IDF v5.5.x

## Build & flash

```bash
./scripts/build_esp32.sh              # configure (once) + build
./scripts/build_esp32.sh flash        # build + flash (default /dev/ttyACM0)
./scripts/build_esp32.sh monitor      # serial monitor
./scripts/build_esp32.sh menuconfig   # edit Kconfig
./scripts/build_esp32.sh fullclean    # wipe build_ttgo_tdisplay + sdkconfig
```

Equivalently, by hand:

```bash
. ~/.espressif/tools/activate_idf_v5.5.5.sh
cd platform/esp32
idf.py -B ../../build_ttgo_tdisplay set-target esp32   # once
idf.py -B ../../build_ttgo_tdisplay build
idf.py -B ../../build_ttgo_tdisplay -p /dev/ttyUSB0 flash monitor
```

## Configuration

All hardware is configured through Kconfig (`main/Kconfig.projbuild`), shown
in `idf.py menuconfig` under **seedmix Hardware**:

| Menu | Options |
|------|---------|
| Display | resolution (presets + custom), controller (ST7789/ILI9341/ST7735/none), SPI pins, RST, backlight, window offset, invert/swap/mirror |
| Touchscreen | enable + controller (XPT2046 SPI / FT5x06 I2C), pins, axis swap/invert |
| Camera | enable + model (OV2640/OV7670), parallel-interface pins |
| Physical buttons | enable, count, per-button GPIO, active level, debounce |
| Entropy / TRNG | hardware RNG (`esp_fill_random`) or disabled |

Preset board configs can live next to `sdkconfig.defaults` (e.g.
`sdkconfig.defaults.ttgo`). To use a preset:

```bash
SDKCONFIG_DEFAULTS="sdkconfig.defaults.ttgo" ./scripts/build_esp32.sh
```

## Hardening

The firmware is hardened:

- **No radio** - the build is configured with `CONFIG_APP_NO_BLOBS=y`, which
  drops the WiFi, Bluetooth and RF-PHY binary blobs, so the device has no
  radio capability and cannot transmit or receive wirelessly.  `main_esp32.c`
  contains a compile-time `#error`, so the build fails if the blobs are
  re-enabled.
- **No flash storage** - the project uses the custom partition table
  `partitions_hardened.csv`, which contains only the `factory` app partition
  (no NVS, no OTA, no PHY-init and no data partition), and PHY calibration
  storage in NVS is disabled (`CONFIG_ESP_PHY_CALIBRATION_AND_DATA_STORAGE=n`).
  NVS is never initialised, so secrets are only ever held in RAM and are wiped
  on power-off.

## What's implemented so far

- Display bring-up via `esp_lcd` ST7789 + LVGL flush callback.
- Physical buttons as an LVGL keypad input device.
- HAL `hal_get_random()` using `esp_fill_random()`
- Camera HAL stubs (gated by `CONFIG_SEEDMIX_CAMERA_ENABLE`).

## Still to do

- Build `libwally` (and its secp256k1) as an ESP-IDF component, plus
  `qrencode`/`quirc`, then compile the shared app sources from `main/`
  (`main.c`, `ui/`, `crypto/`, `qr/`) into this component and call
  `app_init()`.
- Add LVGL focus groups so the two buttons can navigate the UI.
- Implement the touchscreen and camera drivers behind their Kconfig options.
