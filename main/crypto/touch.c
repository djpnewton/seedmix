/**
 * @file main/crypto/touch.c
 * @brief Touch-screen entropy collection.
 */

#include "touch.h"
#include "util/error.h"
#include "util/utils.h"
#include <stdlib.h>

#define TAP_BYTES 4 /* uint16 x + uint16 y */

struct touch_entropy_t {
    uint8_t* acc;
    size_t   acc_len;
    size_t   acc_cap;
    uint32_t bits;
    uint32_t needed;
    uint32_t per_tap;
    unsigned taps;
    size_t   entropy_len; // 16 or 32 bytes
};

touch_entropy_t* touch_entropy_begin(unsigned word_count, uint32_t res_x, uint32_t res_y) {
    ASSERT_OR_DIE(utils_word_count_valid(word_count), "word count must be 12 or 24");
    ASSERT_OR_DIE(res_x > 0 && res_y > 0, "invalid resolution");

    touch_entropy_t* t = calloc(1, sizeof(*t));
    ASSERT_OR_DIE(t, "out of memory");

    t->needed      = utils_word_count_bits(word_count);
    t->entropy_len = utils_word_count_bytes(word_count);

    // A tap contributes log2(width * height) bits (divde by 4 to stay conservative)
    t->per_tap = utils_floor_log2((uint64_t)res_x * res_y) / 4;
    if (t->per_tap == 0) t->per_tap = 1;

    // Worst-case accumulator capacity for the required number of taps
    size_t max_taps = (t->needed + t->per_tap - 1) / t->per_tap;
    t->acc_cap      = max_taps * TAP_BYTES;
    t->acc          = calloc(1, t->acc_cap);
    ASSERT_OR_DIE(t->acc, "out of memory");

    return t;
}

void touch_entropy_add_tap(touch_entropy_t* t, int32_t x, int32_t y) {
    ASSERT_OR_DIE(t, "null touch entropy");
    ASSERT_OR_DIE(t->acc_len + TAP_BYTES <= t->acc_cap, "touch accumulator overflow");

    t->acc[t->acc_len++] = (uint8_t)((uint32_t)x & 0xFF);
    t->acc[t->acc_len++] = (uint8_t)(((uint32_t)x >> 8) & 0xFF);
    t->acc[t->acc_len++] = (uint8_t)((uint32_t)y & 0xFF);
    t->acc[t->acc_len++] = (uint8_t)(((uint32_t)y >> 8) & 0xFF);
    t->taps++;
    t->bits += t->per_tap;
}

bool touch_entropy_ready(const touch_entropy_t* t) {
    ASSERT_OR_DIE(t, "null touch entropy");
    return t->bits >= t->needed;
}

uint32_t touch_entropy_bits(const touch_entropy_t* t) {
    ASSERT_OR_DIE(t, "null touch entropy");
    return t->bits;
}

uint32_t touch_entropy_needed(const touch_entropy_t* t) {
    ASSERT_OR_DIE(t, "null touch entropy");
    return t->needed;
}

unsigned touch_entropy_taps(const touch_entropy_t* t) {
    ASSERT_OR_DIE(t, "null touch entropy");
    return t->taps;
}

size_t touch_entropy_derive(touch_entropy_t* t, uint8_t* out, size_t out_len) {
    ASSERT_OR_DIE(t, "null touch entropy");
    ASSERT_OR_DIE(out, "null out");
    if (!touch_entropy_ready(t)) return 0;

    ASSERT_OR_DIE(out_len >= t->entropy_len, "out buffer too small");
    sha256_expand(t->acc, t->acc_len, out, t->entropy_len);
    return t->entropy_len;
}

void touch_entropy_discard(touch_entropy_t* t) {
    if (!t) return;
    if (t->acc) {
        secure_memzero(t->acc, t->acc_cap);
        free(t->acc);
    }
    free(t);
}
