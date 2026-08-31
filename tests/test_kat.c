/**
 * @file tests/test_kat.c
 * @brief BIP39 known-answer tests (English wordlist, from the BIP39 spec).
 */

#include "crypto/mnemonic.h"
#include "unity.h"

#include <stdint.h>
#include <string.h>

void setUp(void) {}
void tearDown(void) {}

static void kat(const uint8_t* entropy, size_t len, const char* expected) {
    mnemonic_t* m = mnemonic_from_entropy(entropy, len);
    TEST_ASSERT_NOT_NULL(m);
    TEST_ASSERT_EQUAL_STRING(expected, mnemonic_words(m));
    mnemonic_discard(m);
}

static void test_vector_zero_12(void) {
    const uint8_t e[16] = {0};
    kat(e, sizeof(e),
        "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon "
        "about");
}

static void test_vector_7f_12(void) {
    const uint8_t e[16] = {0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
                           0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f};
    kat(e, sizeof(e),
        "legal winner thank year wave sausage worth useful legal winner thank yellow");
}

static void test_vector_80_12(void) {
    const uint8_t e[16] = {0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
                           0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80};
    kat(e, sizeof(e),
        "letter advice cage absurd amount doctor acoustic avoid letter advice cage above");
}

static void test_vector_ff_12(void) {
    const uint8_t e[16] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                           0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    kat(e, sizeof(e), "zoo zoo zoo zoo zoo zoo zoo zoo zoo zoo zoo wrong");
}

static void test_vector_77c2_12(void) {
    const uint8_t e[16] = {0x77, 0xc2, 0xb0, 0x07, 0x16, 0xce, 0xc7, 0x21,
                           0x38, 0x39, 0x15, 0x9e, 0x40, 0x4d, 0xb5, 0x0d};
    kat(e, sizeof(e),
        "jelly better achieve collect unaware mountain thought cargo oxygen act hood bridge");
}

static void test_vector_zero_24(void) {
    const uint8_t e[32] = {0};
    mnemonic_t*   m     = mnemonic_from_entropy(e, sizeof(e));
    TEST_ASSERT_NOT_NULL(m);

    /* 23 x "abandon" + "art" */
    char expected[256];
    expected[0] = '\0';
    for (int i = 0; i < 23; i++) {
        strcat(expected, "abandon ");
    }
    strcat(expected, "art");

    TEST_ASSERT_EQUAL_STRING(expected, mnemonic_words(m));
    mnemonic_discard(m);
}

int main(void) {
    UNITY_BEGIN();
    mnemonic_init();
    RUN_TEST(test_vector_zero_12);
    RUN_TEST(test_vector_7f_12);
    RUN_TEST(test_vector_80_12);
    RUN_TEST(test_vector_ff_12);
    RUN_TEST(test_vector_77c2_12);
    RUN_TEST(test_vector_zero_24);
    return UNITY_END();
}
