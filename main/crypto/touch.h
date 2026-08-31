/**
 * @file main/crypto/touch.h
 * @brief Touch-screen entropy collection.
 *
 * Accumulates tap coordinates until the estimated entropy reaches the amount
 * needed for a 12-word (128-bit) or 24-word (256-bit) mnemonic.
 */

#ifndef TOUCH_H
#define TOUCH_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct touch_entropy_t touch_entropy_t;

/**
 * @brief Start collecting touch entropy.
 *
 * @param word_count 12 or 24.
 * @param res_x      Display width in pixels.
 * @param res_y      Display height in pixels.
 *
 * Each tap is estimated to contribute floor(log2(res_x * res_y))
 *
 * @return New collector (caller owns it; discard with touch_entropy_discard()).
 */
touch_entropy_t* touch_entropy_begin(unsigned word_count, uint32_t res_x, uint32_t res_y);

/** Add one tap coordinate to the accumulator. */
void touch_entropy_add_tap(touch_entropy_t* t, int32_t x, int32_t y);

/** True once the estimated entropy meets the target. */
bool touch_entropy_ready(const touch_entropy_t* t);

/** Estimated bits collected so far. */
uint32_t touch_entropy_bits(const touch_entropy_t* t);

/** Estimated bits required (128 or 256). */
uint32_t touch_entropy_needed(const touch_entropy_t* t);

/** Number of taps collected so far. */
unsigned touch_entropy_taps(const touch_entropy_t* t);

/**
 * @brief Derive the mnemonic entropy from the collected taps.
 *
 * @param t       Collector (must be ready).
 * @param out     Buffer of at least 32 bytes.
 * @param out_len Capacity of @p out.
 * @return        Bytes written (16 or 32), or 0 if not ready.
 */
size_t touch_entropy_derive(touch_entropy_t* t, uint8_t* out, size_t out_len);

/** Discard (zero + free). */
void touch_entropy_discard(touch_entropy_t* t);

#ifdef __cplusplus
}
#endif

#endif /* TOUCH_H */
