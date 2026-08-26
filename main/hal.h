/**
 * @file main/hal.h
 * @brief Hardware abstraction layer - platform-specific primitives.
 *
 * Each supported platform provides an implementation of these functions
 * (see platform/<platform>/hal_<platform>.c).  Shared application code
 * should only ever use this header - never platform APIs directly.
 */

#ifndef HAL_H
#define HAL_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Get the name of the entropy source
 */
const char* hal_get_random_source();

/**
 * @brief Fill @p buf with @p len cryptographically secure random bytes.
 *
 * The entropy source is platform-specific (e.g. /dev/urandom on Linux,
 * the hardware TRNG on ESP32).  On failure this function does not return.
 *
 * @param buf  Output buffer (must not be NULL).
 * @param len  Number of bytes to fill (must be > 0).
 */
void hal_get_random(uint8_t* buf, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* HAL_H */
