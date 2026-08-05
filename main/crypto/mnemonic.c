/**
 * @file main/crypto/mnemonic.c
 * @brief BIP39 mnemonic generation.
 */

#include "mnemonic.h"
#include "secure_stack.h"
#include "util/error.h"
#include "util/log.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <wally_core.h>
#include <wally_bip39.h>

/* -- mnemonic_t definition -------------------------------------------- */
struct mnemonic_t {
    char            *words;        /* null-terminated mnemonic string */
    secure_stack_t  *stack;        /* tracks sensitive data for zeroing */
};

const char *mnemonic_words(const mnemonic_t *m) {
    return m ? m->words : NULL;
}

/* -- Init (idempotent) ------------------------------------------------ */
void mnemonic_init(void) {
    static bool done = false;
    if (done) return;
    int rc = wally_init(0);
    ASSERT_OR_DIE(rc == WALLY_OK, "wally_init failed");
    done = true;
    LOG_INFO("libwally initialized");
}

/* -- Generate --------------------------------------------------------- */
mnemonic_t *mnemonic_generate(void) {
    unsigned char entropy[16];
    int rc;

    FILE *urandom = fopen("/dev/urandom", "rb");
    ASSERT_OR_DIE(urandom, "Failed to open /dev/urandom");
    size_t n = fread(entropy, 1, sizeof(entropy), urandom);
    ASSERT_OR_DIE(n == sizeof(entropy), "Short read from /dev/urandom");
    fclose(urandom);

    char *words = NULL;
    rc = bip39_mnemonic_from_bytes(NULL, entropy, sizeof(entropy), &words);
    ASSERT_OR_DIE(rc == WALLY_OK, "bip39_mnemonic_from_bytes failed");
    wally_bzero(entropy, sizeof(entropy));

    mnemonic_t *m = calloc(1, sizeof(*m));
    ASSERT_OR_DIE(m, "mnemonic_generate: out of memory");

    m->words = words;
    m->stack = secure_stack_create(1);
    ASSERT_OR_DIE(m->stack, "mnemonic_generate: out of memory");
    secure_stack_push(m->stack, (uint8_t *)words, strlen(words) + 1);

    LOG_INFO("Mnemonic generated");
    return m;
}

/* -- Discard ---------------------------------------------------------- */
void mnemonic_discard(mnemonic_t *m) {
    if (!m) return;
    if (m->words) {
        secure_stack_pop(m->stack, (uint8_t *)m->words);
        free(m->words);
    }
    secure_stack_destroy(m->stack);
    free(m);
    LOG_INFO("mnemonic zeroed and freed");
}
