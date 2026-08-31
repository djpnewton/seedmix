/**
 * @file fuzz/fuzz_mnemonic.c
 * @brief libFuzzer target for mnemonic_from_string().
 */

#include "crypto/mnemonic.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    mnemonic_init();

    if (size == 0 || size > 4096) return 0;

    char* buf = (char*)malloc(size + 1);
    if (!buf) return 0;
    memcpy(buf, data, size);
    buf[size] = '\0';

    mnemonic_t* m = mnemonic_from_string(buf);
    if (m) mnemonic_discard(m);

    free(buf);
    return 0;
}
