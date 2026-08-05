/**
 * @file main/crypto/mnemonic.h
 * @brief BIP39 mnemonic generation using libwally.
 */

#ifndef MNEMONIC_H
#define MNEMONIC_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/** A generated mnemonic.  Discard with mnemonic_discard() to zero it. */
typedef struct mnemonic_t mnemonic_t;

/** Access the words (read-only). */
const char *mnemonic_words(const mnemonic_t *m);

/**
 * @brief Initialize libwally.
 */
void mnemonic_init(void);

/**
 * @brief Generate a BIP39 mnemonic (12 words).  The returned mnemonic
 *        owns a secure stack and must be discarded with mnemonic_discard().
 */
mnemonic_t *mnemonic_generate(void);

/**
 * @brief Zero the mnemonic, pop it from its secure stack, destroy the
 *        stack, and free all associated memory.
 */
void mnemonic_discard(mnemonic_t *m);

#ifdef __cplusplus
}
#endif

#endif /* MNEMONIC_H */
