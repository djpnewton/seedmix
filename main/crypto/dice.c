/**
 * @file main/crypto/dice.c
 * @brief Dice-roll entropy collection.
 */

#include "dice.h"
#include "util/error.h"
#include "util/utils.h"
#include <stdlib.h>

struct dice_entropy_t {
    uint8_t* rolls;
    size_t   len;
    size_t   cap;
    uint32_t bits;
    uint32_t needed;
    uint32_t per_roll;
    unsigned count;
    unsigned sides;
    size_t   entropy_len; // 16 or 32 bytes
};

dice_entropy_t* dice_entropy_begin(unsigned word_count, unsigned sides) {
    ASSERT_OR_DIE(utils_word_count_valid(word_count), "word count must be 12 or 24");
    ASSERT_OR_DIE(sides >= 2 && sides <= 255, "dice sides must be between 2 and 255");

    dice_entropy_t* d = calloc(1, sizeof(*d));
    ASSERT_OR_DIE(d, "out of memory");

    d->sides       = sides;
    d->needed      = utils_word_count_bits(word_count);
    d->entropy_len = utils_word_count_bytes(word_count);

    // Conservative per-roll estimate: floor(log2(sides)).
    d->per_roll = utils_floor_log2(sides);
    if (d->per_roll == 0) d->per_roll = 1;

    // Worst-case accumulator capacity for the required number of rolls.
    d->cap   = (d->needed + d->per_roll - 1) / d->per_roll;
    d->rolls = calloc(1, d->cap);
    ASSERT_OR_DIE(d->rolls, "out of memory");

    return d;
}

void dice_entropy_add_roll(dice_entropy_t* d, unsigned value) {
    ASSERT_OR_DIE(d, "null dice entropy");
    ASSERT_OR_DIE(value >= 1 && value <= d->sides, "dice value out of range");
    ASSERT_OR_DIE(d->len + 1 <= d->cap, "dice accumulator overflow");

    d->rolls[d->len++] = (uint8_t)value;
    d->count++;
    d->bits += d->per_roll;
}

bool dice_entropy_ready(const dice_entropy_t* d) {
    ASSERT_OR_DIE(d, "null dice entropy");
    return d->bits >= d->needed;
}

uint32_t dice_entropy_bits(const dice_entropy_t* d) {
    ASSERT_OR_DIE(d, "null dice entropy");
    return d->bits;
}

uint32_t dice_entropy_needed(const dice_entropy_t* d) {
    ASSERT_OR_DIE(d, "null dice entropy");
    return d->needed;
}

unsigned dice_entropy_rolls(const dice_entropy_t* d) {
    ASSERT_OR_DIE(d, "null dice entropy");
    return d->count;
}

unsigned dice_entropy_sides(const dice_entropy_t* d) {
    ASSERT_OR_DIE(d, "null dice entropy");
    return d->sides;
}

size_t dice_entropy_derive(dice_entropy_t* d, uint8_t* out, size_t out_len) {
    ASSERT_OR_DIE(d, "null dice entropy");
    ASSERT_OR_DIE(out, "null out");
    if (!dice_entropy_ready(d)) return 0;

    ASSERT_OR_DIE(out_len >= d->entropy_len, "out buffer too small");
    sha256_expand(d->rolls, d->len, out, d->entropy_len);
    return d->entropy_len;
}

void dice_entropy_discard(dice_entropy_t* d) {
    if (!d) return;
    if (d->rolls) {
        secure_memzero(d->rolls, d->cap);
        free(d->rolls);
    }
    free(d);
}
