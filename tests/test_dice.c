/**
 * @file tests/test_dice.c
 * @brief Unity tests for main/crypto/dice.c
 */

#include "crypto/dice.h"
#include "unity.h"

#include <stdint.h>

void setUp(void) {}
void tearDown(void) {}

static void test_begin_defaults(void) {
    dice_entropy_t* d = dice_entropy_begin(12, 6);
    TEST_ASSERT_NOT_NULL(d);
    TEST_ASSERT_EQUAL_UINT(6, dice_entropy_sides(d));
    TEST_ASSERT_EQUAL_UINT(128, dice_entropy_needed(d));
    TEST_ASSERT_EQUAL_UINT(0, dice_entropy_bits(d));
    TEST_ASSERT_EQUAL_UINT(0, dice_entropy_rolls(d));
    TEST_ASSERT_FALSE(dice_entropy_ready(d));
    dice_entropy_discard(d);
}

static void test_accumulate_until_ready(void) {
    // 6 sides -> floor(log2(6)) = 2 bits per roll; 64 rolls -> 128 bits
    dice_entropy_t* d = dice_entropy_begin(12, 6);
    for (unsigned i = 0; i < 63; i++) dice_entropy_add_roll(d, (i % 6) + 1);
    TEST_ASSERT_FALSE(dice_entropy_ready(d));
    TEST_ASSERT_EQUAL_UINT(126, dice_entropy_bits(d));

    dice_entropy_add_roll(d, 1);
    TEST_ASSERT_TRUE(dice_entropy_ready(d));
    TEST_ASSERT_EQUAL_UINT(64, dice_entropy_rolls(d));
    dice_entropy_discard(d);
}

static void test_derive_not_ready_returns_zero(void) {
    dice_entropy_t* d       = dice_entropy_begin(12, 6);
    uint8_t         out[32] = {0};
    TEST_ASSERT_EQUAL_UINT(0, (unsigned)dice_entropy_derive(d, out, sizeof(out)));
    dice_entropy_discard(d);
}

static void test_derive_length_16(void) {
    dice_entropy_t* d = dice_entropy_begin(12, 6);
    for (unsigned i = 0; i < 64; i++) dice_entropy_add_roll(d, (i % 6) + 1);
    uint8_t out[32] = {0};
    TEST_ASSERT_EQUAL_UINT(16, (unsigned)dice_entropy_derive(d, out, sizeof(out)));
    dice_entropy_discard(d);
}

static void test_derive_length_32(void) {
    dice_entropy_t* d = dice_entropy_begin(24, 6);
    for (unsigned i = 0; i < 128; i++) dice_entropy_add_roll(d, (i % 6) + 1);
    uint8_t out[32] = {0};
    TEST_ASSERT_EQUAL_UINT(32, (unsigned)dice_entropy_derive(d, out, sizeof(out)));
    dice_entropy_discard(d);
}

static void test_derive_deterministic(void) {
    dice_entropy_t* a = dice_entropy_begin(12, 6);
    dice_entropy_t* b = dice_entropy_begin(12, 6);
    for (unsigned i = 0; i < 64; i++) {
        unsigned v = (i % 6) + 1;
        dice_entropy_add_roll(a, v);
        dice_entropy_add_roll(b, v);
    }
    uint8_t outa[16] = {0}, outb[16] = {0};
    TEST_ASSERT_EQUAL_UINT(16, (unsigned)dice_entropy_derive(a, outa, sizeof(outa)));
    TEST_ASSERT_EQUAL_UINT(16, (unsigned)dice_entropy_derive(b, outb, sizeof(outb)));
    TEST_ASSERT_EQUAL_MEMORY(outa, outb, sizeof(outa));
    dice_entropy_discard(a);
    dice_entropy_discard(b);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_begin_defaults);
    RUN_TEST(test_accumulate_until_ready);
    RUN_TEST(test_derive_not_ready_returns_zero);
    RUN_TEST(test_derive_length_16);
    RUN_TEST(test_derive_length_32);
    RUN_TEST(test_derive_deterministic);
    return UNITY_END();
}
