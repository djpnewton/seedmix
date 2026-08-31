/**
 * @file fuzz/fuzz_mnemonic_combine.c
 * @brief libFuzzer property test: combine(a, b) entropy == entropy(a) ^ entropy(b).
 */

#include "crypto/mnemonic.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    mnemonic_init();

    /* Need two 16-byte (or two 32-byte) entropies. */
    if (size < 32) return 0;
    size_t len = (size >= 64 && (data[0] & 1)) ? 32 : 16;

    uint8_t a[32], b[32];
    memcpy(a, data, len);
    memcpy(b, data + len, len);

    /* combine() refuses identical entropy (intentional FATAL); guarantee a != b. */
    if (memcmp(a, b, len) == 0) b[0] ^= 0xFF;

    mnemonic_t* ma = mnemonic_from_entropy(a, len);
    mnemonic_t* mb = mnemonic_from_entropy(b, len);
    mnemonic_t* mc = mnemonic_combine(ma, mb); /* discards ma, mb */

    uint8_t expected[32];
    for (size_t i = 0; i < len; i++) expected[i] = a[i] ^ b[i];

    uint8_t out[32] = {0};
    size_t  n       = mnemonic_to_entropy(mc, out);
    if (n != len || memcmp(expected, out, len) != 0) abort();

    mnemonic_discard(mc);
    return 0;
}
