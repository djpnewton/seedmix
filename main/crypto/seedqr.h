/**
 * @file main/crypto/seedqr.h
 * @brief SeedQR (Standard + Compact) encode/decode of BIP39 mnemonics.
 *
 * Format reference: SeedSigner docs/seed_qr/README.md
 *  - Standard SeedQR: zero-padded 4-digit BIP39 word indices concatenated
 *    (48 chars for 12 words, 96 chars for 24 words).
 *  - CompactSeedQR: the raw entropy bytes (16 or 32).
 *
 * These functions produce/consume only the *payload*; actual QR bitmap
 * generation/scanning lives in the UI/HAL layers.
 */

#ifndef SEEDQR_H
#define SEEDQR_H

#include <stddef.h>
#include <stdint.h>

#include "mnemonic.h"

#define SEEDQR_STANDARD_12_DIGITS 48
#define SEEDQR_STANDARD_24_DIGITS 96

/** Encode the mnemonic's entropy bytes (CompactSeedQR payload). Returns 16 or 32, or 0. */
size_t seedqr_compact_encode(const mnemonic_t* m, uint8_t* out, size_t out_cap);

/** Decode 16/32 entropy bytes into a mnemonic, or NULL on invalid input. */
mnemonic_t* seedqr_compact_decode(const uint8_t* entropy, size_t len);

/** Encode the mnemonic as a Standard SeedQR digit stream. Returns 48 or 96, or 0. */
size_t seedqr_standard_encode(const mnemonic_t* m, char* out, size_t out_cap);

/** Decode a 48/96-char Standard SeedQR digit stream, or NULL on invalid input. */
mnemonic_t* seedqr_standard_decode(const char* digits);

#endif /* SEEDQR_H */
