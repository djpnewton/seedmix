/**
 * @file main/crypto/bip39_wordlist.c
 * @brief BIP39 word list prefix search using libwally.
 */

#include "bip39_wordlist.h"
#include "util/error.h"

#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <wally_bip39.h>

static const char* wordlist[BIP39_WORD_COUNT];
static bool        loaded = false;

void bip39_wordlist_init(void) {
    if (loaded) return;
    struct words* wl = NULL;
    ASSERT_OR_DIE(bip39_get_wordlist(NULL, &wl) == WALLY_OK, "bip39_get_wordlist failed");
    for (size_t i = 0; i < BIP39_WORD_COUNT; i++) {
        wordlist[i] = bip39_get_word_by_index(wl, i);
        ASSERT_OR_DIE(wordlist[i], "wordlist index out of range");
    }
    loaded = true;
}

static bool prefix_match(const char* word, const char* prefix, size_t plen) {
    if (plen == 0) return false;
    for (size_t i = 0; i < plen; i++) {
        if (tolower((unsigned char)word[i]) != tolower((unsigned char)prefix[i])) return false;
    }
    return true;
}

size_t bip39_wordlist_lookup(const char* prefix, const char** matches) {
    if (!prefix || !*prefix) return 0;
    size_t plen  = strlen(prefix);
    size_t count = 0;
    for (size_t i = 0; i < BIP39_WORD_COUNT && count < BIP39_MAX_MATCHES; i++) {
        if (prefix_match(wordlist[i], prefix, plen)) {
            matches[count++] = wordlist[i];
        }
    }
    return count;
}

const char* bip39_wordlist_word(size_t index) {
    if (index >= BIP39_WORD_COUNT) return NULL;
    bip39_wordlist_init();
    return wordlist[index];
}

size_t bip39_wordlist_index(const char* word) {
    if (!word || !*word) return SIZE_MAX;
    bip39_wordlist_init();
    for (size_t i = 0; i < BIP39_WORD_COUNT; i++) {
        if (strcmp(wordlist[i], word) == 0) return i;
    }
    return SIZE_MAX;
}
