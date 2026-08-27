/**
 * @file main/util/utils.h
 * @brief General utility functions.
 */

#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Format bytes as lowercase hex.
 * @param data      Input bytes.
 * @param len       Number of bytes.
 * @param out       Output buffer (at least len * 2 + 1 chars).
 * @param out_size  Size of `out` in bytes.
 */
void bytes_to_hex(const uint8_t* data, size_t len, char* out, size_t out_size);

/**
 * @brief Deterministically expand @p data into @p out_len bytes using
 *        SHA-256 in counter mode (works for any output length).
 *
 * The first 32-byte block is SHA-256(data); each subsequent block is
 * SHA-256(counter || seed).  This is an expansion, not a KDF.
 *
 * @param data      Input bytes.
 * @param data_len  Number of input bytes.
 * @param out       Output buffer (at least @p out_len bytes).
 * @param out_len   Number of bytes to produce.
 */
void sha256_expand(const uint8_t* data, size_t data_len, uint8_t* out, size_t out_len);

#ifdef __cplusplus
}
#endif

#endif /* UTILS_H */
