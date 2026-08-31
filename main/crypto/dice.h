/**
 * @file main/crypto/dice.h
 * @brief Dice-roll entropy collection.
 *
 * Accumulates die roll values (1..sides) until the estimated entropy reaches
 * the amount needed for a 12-word (128-bit) or 24-word (256-bit) mnemonic.
 */

#ifndef DICE_H
#define DICE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct dice_entropy_t dice_entropy_t;

/**
 * @brief Start collecting dice-roll entropy.
 *
 * @param word_count 12 or 24.
 * @param sides      Number of die faces (>= 2).
 *
 * Each roll is estimated to contribute floor(log2(sides)) bits.
 *
 * @return New collector (caller owns it; discard with dice_entropy_discard()).
 */
dice_entropy_t* dice_entropy_begin(unsigned word_count, unsigned sides);

/** Add one die roll (value 1..sides) to the accumulator. */
void dice_entropy_add_roll(dice_entropy_t* d, unsigned value);

/** Number of die faces. */
unsigned dice_entropy_sides(const dice_entropy_t* d);

/** True once the estimated entropy meets the target. */
bool dice_entropy_ready(const dice_entropy_t* d);

/** Estimated bits collected so far. */
uint32_t dice_entropy_bits(const dice_entropy_t* d);

/** Estimated bits required (128 or 256). */
uint32_t dice_entropy_needed(const dice_entropy_t* d);

/** Number of rolls collected so far. */
unsigned dice_entropy_rolls(const dice_entropy_t* d);

/**
 * @brief Derive the mnemonic entropy from the collected rolls.
 *
 * @param d       Collector (must be ready).
 * @param out     Buffer of at least 32 bytes.
 * @param out_len Capacity of @p out.
 * @return        Bytes written (16 or 32), or 0 if not ready.
 */
size_t dice_entropy_derive(dice_entropy_t* d, uint8_t* out, size_t out_len);

/** Discard (zero + free). */
void dice_entropy_discard(dice_entropy_t* d);

#ifdef __cplusplus
}
#endif

#endif /* DICE_H */
