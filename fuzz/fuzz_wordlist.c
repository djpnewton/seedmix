/**
 * @file fuzz/fuzz_wordlist.c
 * @brief libFuzzer target for bip39_wordlist_lookup().
 */

#include "crypto/bip39_wordlist.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    static bool initialized = false;
    if (!initialized) {
        bip39_wordlist_init();
        initialized = true;
    }

    if (size == 0 || size > 4096) return 0;

    char* buf = (char*)malloc(size + 1);
    if (!buf) return 0;
    memcpy(buf, data, size);
    buf[size] = '\0';

    const char* matches[BIP39_MAX_MATCHES] = {0};
    (void)bip39_wordlist_lookup(buf, matches);

    free(buf);
    return 0;
}
