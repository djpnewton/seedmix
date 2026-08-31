/**
 * @file tests/test_touch.c
 * @brief Unity tests for main/crypto/touch.c
 */

#include "crypto/touch.h"
#include "unity.h"

#include <stdint.h>

void setUp(void) {}
void tearDown(void) {}

static void test_begin_defaults(void) {
    touch_entropy_t* t = touch_entropy_begin(12, 480, 320);
    TEST_ASSERT_NOT_NULL(t);
    TEST_ASSERT_EQUAL_UINT(128, touch_entropy_needed(t));
    TEST_ASSERT_EQUAL_UINT(0, touch_entropy_bits(t));
    TEST_ASSERT_EQUAL_UINT(0, touch_entropy_taps(t));
    TEST_ASSERT_FALSE(touch_entropy_ready(t));
    touch_entropy_discard(t);
}

static void test_accumulate_until_ready(void) {
    // 480x320 -> floor_log2(153600)=17 -> per_tap = 17/4 = 4 bits.
    touch_entropy_t* t = touch_entropy_begin(12, 480, 320);
    for (unsigned i = 0; i < 31; i++) {
        touch_entropy_add_tap(t, (int32_t)i, (int32_t)(i * 2));
    }
    TEST_ASSERT_FALSE(touch_entropy_ready(t));
    TEST_ASSERT_EQUAL_UINT(124, touch_entropy_bits(t));
    TEST_ASSERT_EQUAL_UINT(31, touch_entropy_taps(t));

    touch_entropy_add_tap(t, 100, 200);
    TEST_ASSERT_TRUE(touch_entropy_ready(t));
    TEST_ASSERT_EQUAL_UINT(128, touch_entropy_bits(t));
    TEST_ASSERT_EQUAL_UINT(32, touch_entropy_taps(t));
    touch_entropy_discard(t);
}

static void test_small_resolution_min_one_bit(void) {
    touch_entropy_t* t = touch_entropy_begin(12, 1, 1); // per_tap clamps to 1
    TEST_ASSERT_NOT_NULL(t);
    touch_entropy_add_tap(t, 0, 0);
    TEST_ASSERT_EQUAL_UINT(1, touch_entropy_bits(t));
    touch_entropy_discard(t);
}

static void test_derive_lengths(void) {
    uint8_t out[32] = {0};

    touch_entropy_t* t12 = touch_entropy_begin(12, 480, 320);
    TEST_ASSERT_EQUAL_UINT(0, (unsigned)touch_entropy_derive(t12, out, sizeof(out)));
    for (unsigned i = 0; i < 32; i++) {
        touch_entropy_add_tap(t12, (int32_t)i, (int32_t)i);
    }
    TEST_ASSERT_EQUAL_UINT(16, (unsigned)touch_entropy_derive(t12, out, sizeof(out)));
    touch_entropy_discard(t12);

    touch_entropy_t* t24 = touch_entropy_begin(24, 480, 320);
    for (unsigned i = 0; i < 64; i++) {
        touch_entropy_add_tap(t24, (int32_t)i, (int32_t)i);
    }
    TEST_ASSERT_TRUE(touch_entropy_ready(t24));
    TEST_ASSERT_EQUAL_UINT(32, (unsigned)touch_entropy_derive(t24, out, sizeof(out)));
    touch_entropy_discard(t24);
}

static void test_derive_deterministic(void) {
    touch_entropy_t* a = touch_entropy_begin(12, 480, 320);
    touch_entropy_t* b = touch_entropy_begin(12, 480, 320);
    for (unsigned i = 0; i < 32; i++) {
        touch_entropy_add_tap(a, (int32_t)i, (int32_t)(i + 1));
        touch_entropy_add_tap(b, (int32_t)i, (int32_t)(i + 1));
    }
    uint8_t outa[16] = {0}, outb[16] = {0};
    TEST_ASSERT_EQUAL_UINT(16, (unsigned)touch_entropy_derive(a, outa, sizeof(outa)));
    TEST_ASSERT_EQUAL_UINT(16, (unsigned)touch_entropy_derive(b, outb, sizeof(outb)));
    TEST_ASSERT_EQUAL_MEMORY(outa, outb, sizeof(outa));
    touch_entropy_discard(a);
    touch_entropy_discard(b);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_begin_defaults);
    RUN_TEST(test_accumulate_until_ready);
    RUN_TEST(test_small_resolution_min_one_bit);
    RUN_TEST(test_derive_lengths);
    RUN_TEST(test_derive_deterministic);
    return UNITY_END();
}
