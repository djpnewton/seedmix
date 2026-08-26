/**
 * @file main/crypto/bip39_wordlist.h
 * @brief BIP39 word list autocomplete using libwally.
 */

#ifndef BIP39_WORDLIST_H
#define BIP39_WORDLIST_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BIP39_WORD_COUNT 2048
#define BIP39_MAX_MATCHES 20

/** Load the word list (must call before using other functions). */
void bip39_wordlist_init(void);

/**
 * @brief Find words matching a prefix (case-insensitive).
 * @param prefix  The typed prefix (e.g. "aba").
 * @param matches Output array of matching word pointers (BIP39_MAX_MATCHES max).
 * @return Number of matches found.
 */
size_t bip39_wordlist_lookup(const char* prefix, const char** matches);

#ifdef __cplusplus
}
#endif

#endif /* BIP39_WORDLIST_H */
