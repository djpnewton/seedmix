/**
 * @file tests/test_utils.c
 * @brief Unity tests for main/util/utils.c
 */

#include "unity.h"
#include "util/utils.h"

#include <stdint.h>
#include <string.h>

void setUp(void) {}
void tearDown(void) {}

static void test_word_count_valid(void) {
    TEST_ASSERT_TRUE(utils_word_count_valid(12));
    TEST_ASSERT_TRUE(utils_word_count_valid(24));
    TEST_ASSERT_FALSE(utils_word_count_valid(11));
    TEST_ASSERT_FALSE(utils_word_count_valid(18));
    TEST_ASSERT_FALSE(utils_word_count_valid(0));
}

static void test_word_count_bits_and_bytes(void) {
    TEST_ASSERT_EQUAL_UINT(128, utils_word_count_bits(12));
    TEST_ASSERT_EQUAL_UINT(256, utils_word_count_bits(24));
    TEST_ASSERT_EQUAL_UINT(16, (unsigned)utils_word_count_bytes(12));
    TEST_ASSERT_EQUAL_UINT(32, (unsigned)utils_word_count_bytes(24));
}

static void test_floor_log2(void) {
    TEST_ASSERT_EQUAL_UINT(0, utils_floor_log2(1));
    TEST_ASSERT_EQUAL_UINT(1, utils_floor_log2(2));
    TEST_ASSERT_EQUAL_UINT(1, utils_floor_log2(3));
    TEST_ASSERT_EQUAL_UINT(2, utils_floor_log2(4));
    TEST_ASSERT_EQUAL_UINT(2, utils_floor_log2(7));
    TEST_ASSERT_EQUAL_UINT(3, utils_floor_log2(8));
    TEST_ASSERT_EQUAL_UINT(32, utils_floor_log2(UINT64_C(1) << 32));
    TEST_ASSERT_EQUAL_UINT(63, utils_floor_log2(UINT64_MAX));
}

static void test_bytes_to_hex(void) {
    const uint8_t data[4] = {0xde, 0xad, 0xbe, 0xef};
    char          out[9]  = {0};
    bytes_to_hex(data, sizeof(data), out, sizeof(out));
    TEST_ASSERT_EQUAL_STRING("deadbeef", out);
}

static void test_sha256_expand_deterministic_and_prefix(void) {
    const uint8_t seed[1] = {0x42};
    uint8_t       out16[16];
    uint8_t       out32[32];
    uint8_t       out40[40];
    uint8_t       again16[16];

    sha256_expand(seed, sizeof(seed), out16, sizeof(out16));
    sha256_expand(seed, sizeof(seed), out32, sizeof(out32));
    sha256_expand(seed, sizeof(seed), out40, sizeof(out40));
    sha256_expand(seed, sizeof(seed), again16, sizeof(again16));

    TEST_ASSERT_EQUAL_MEMORY(out16, again16, sizeof(out16));
    TEST_ASSERT_EQUAL_MEMORY(out32, out16, sizeof(out16));
    TEST_ASSERT_EQUAL_MEMORY(out40, out32, sizeof(out32));
}

static void test_sha256_expand_known_vector(void) {
    // Block 0 is SHA-256( 0x00000000 || SHA-256(seed) )
    const uint8_t seed[3]      = {'a', 'b', 'c'};
    const uint8_t expected[32] = {
        0xf5, 0x5a, 0xe1, 0x0f, 0x6d, 0xb6, 0xa1, 0x7e, 0xf3, 0x78, 0x5a,
        0x68, 0x69, 0x3f, 0xf9, 0xbd, 0x7f, 0xf8, 0x4d, 0x32, 0x39, 0xd2,
        0xf9, 0x3f, 0x02, 0xeb, 0x6e, 0x5e, 0x13, 0xdf, 0x90, 0xe2,
    };
    uint8_t out[32] = {0};
    sha256_expand(seed, sizeof(seed), out, sizeof(out));
    TEST_ASSERT_EQUAL_MEMORY(expected, out, sizeof(expected));
}

static void test_secure_memzero(void) {
    uint8_t buf[16];
    memset(buf, 0xAA, sizeof(buf));
    secure_memzero(buf, sizeof(buf));
    for (size_t i = 0; i < sizeof(buf); i++) {
        TEST_ASSERT_EQUAL_UINT8(0, buf[i]);
    }
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_word_count_valid);
    RUN_TEST(test_word_count_bits_and_bytes);
    RUN_TEST(test_floor_log2);
    RUN_TEST(test_bytes_to_hex);
    RUN_TEST(test_sha256_expand_deterministic_and_prefix);
    RUN_TEST(test_sha256_expand_known_vector);
    RUN_TEST(test_secure_memzero);
    return UNITY_END();
}
