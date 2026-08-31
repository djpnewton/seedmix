/**
 * @file fuzz/fuzz_mnemonic_roundtrip.c
 * @brief libFuzzer property test: entropy -> mnemonic -> entropy roundtrip.
 */

#include "crypto/mnemonic.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    mnemonic_init();

    if (size < 16) return 0;
    /* Exercise both 12-word (16 bytes) and 24-word (32 bytes) paths. */
    size_t len = (size >= 32 && (data[0] & 1)) ? 32 : 16;

    uint8_t entropy[32];
    memcpy(entropy, data, len);

    mnemonic_t* m = mnemonic_from_entropy(entropy, len);

    uint8_t out[32] = {0};
    size_t  n       = mnemonic_to_entropy(m, out);
    if (n != len || memcmp(entropy, out, len) != 0) abort();

    mnemonic_discard(m);
    return 0;
}
