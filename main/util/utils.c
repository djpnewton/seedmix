/**
 * @file main/util/utils.c
 * @brief General utility functions.
 */

#include "utils.h"
#include "error.h"
#include <stdio.h>
#include <string.h>

#include <wally_core.h>
#include <wally_crypto.h>

void bytes_to_hex(const uint8_t* data, size_t len, char* out, size_t out_size) {
    ASSERT_OR_DIE(data && len > 0, "invalid data");
    ASSERT_OR_DIE(out, "null output buffer");
    ASSERT_OR_DIE(out_size >= len * 2 + 1, "hex buffer too small");
    for (size_t i = 0; i < len; i++) {
        int res = snprintf(out + i * 2, 3, "%02x", data[i]);
        ASSERT_OR_DIE(res > 0 && (size_t)res < 3, "hex formatting failed");
    }
    out[len * 2] = '\0';
}

void sha256_expand(const uint8_t* data, size_t data_len, uint8_t* out, size_t out_len) {
    ASSERT_OR_DIE(data && data_len > 0, "sha256_expand: invalid input");
    ASSERT_OR_DIE(out && out_len > 0, "sha256_expand: invalid output");

    uint8_t seed[SHA256_LEN];
    ASSERT_OR_DIE(wally_sha256(data, data_len, seed, SHA256_LEN) == WALLY_OK,
                  "sha256_expand: SHA-256 failed");

    uint32_t counter = 0;
    size_t   off     = 0;
    while (off < out_len) {
        uint8_t block[4 + SHA256_LEN];
        block[0] = (uint8_t)(counter >> 24);
        block[1] = (uint8_t)(counter >> 16);
        block[2] = (uint8_t)(counter >> 8);
        block[3] = (uint8_t)(counter);
        memcpy(block + 4, seed, SHA256_LEN);

        uint8_t h[SHA256_LEN];
        ASSERT_OR_DIE(wally_sha256(block, sizeof(block), h, SHA256_LEN) == WALLY_OK,
                      "sha256_expand: SHA-256 failed");

        size_t n = out_len - off;
        if (n > SHA256_LEN) n = SHA256_LEN;
        memcpy(out + off, h, n);
        off += n;
        counter++;
    }

    wally_bzero(seed, sizeof(seed));
}
