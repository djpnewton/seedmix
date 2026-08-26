/**
 * @file main/crypto/mnemonic.c
 * @brief BIP39 mnemonic generation, parsing, and entropy combining.
 */

#include "mnemonic.h"
#include "secure_stack.h"
#include "util/error.h"
#include "util/log.h"
#include "util/utils.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <wally_core.h>
#include <wally_bip39.h>

#define MAX_ENTROPY_BYTES 32

/* -- Helpers ---------------------------------------------------------- */
struct mnemonic_t {
    char* words;
    secure_stack_t* stack;
    size_t entropy_len;  /* 16 or 32 */
};

static bool word_count_valid(unsigned wc) { return wc == 12 || wc == 24; }

static size_t word_count_to_bytes(unsigned wc) {
    ASSERT_OR_DIE(word_count_valid(wc), "word count not valid");
    return wc == 12 ? 16 : 32;
}

static size_t entropy_len_valid(size_t len) { return len == 16 || len == 32; }

static void get_random(uint8_t *buf, size_t len, mnemonic_process_cb_t process_cb) {
    ASSERT_OR_DIE(buf && len > 0, "invalid buffer");
    ASSERT_OR_DIE(process_cb, "process callback is required");

    char process[64];
    int res = snprintf(process, sizeof(process), "Generating %zu bytes of entropy from /dev/urandom...", len);
    ASSERT_OR_DIE(res > 0 && (size_t)res < sizeof(process), "process string too long");
    process_cb(process);
    FILE *f = fopen("/dev/urandom", "rb");
    ASSERT_OR_DIE(f, "Failed to open /dev/urandom");
    size_t n = fread(buf, 1, len, f);
    ASSERT_OR_DIE(n == len, "Short read from /dev/urandom");
    fclose(f);
}

static mnemonic_t* mnemonic_alloc(char* words, size_t entropy_len) {
    ASSERT_OR_DIE(words, "null words");
    ASSERT_OR_DIE(entropy_len_valid(entropy_len), "entropy length must be 16 or 32");

    mnemonic_t* m = calloc(1, sizeof(*m));
    ASSERT_OR_DIE(m, "out of memory");
    m->words       = words;
    m->entropy_len = entropy_len;
    m->stack       = secure_stack_create(2);
    ASSERT_OR_DIE(m->stack, "out of memory");
    secure_stack_push(m->stack, (uint8_t *)words, strlen(words) + 1);
    return m;
}

/* -- Public API ------------------------------------------------------- */

void mnemonic_init(void) {
    static bool done = false;
    if (done) return;
    ASSERT_OR_DIE(wally_init(0) == WALLY_OK, "wally_init failed");
    done = true;
    LOG_INFO("libwally initialized");
}

const char* mnemonic_words(const mnemonic_t* m) { return m ? m->words : NULL; }

size_t mnemonic_entropy_size(const mnemonic_t* m) { return m ? m->entropy_len : 0; }

mnemonic_t* mnemonic_generate(unsigned word_count, mnemonic_process_cb_t process_cb) {
    ASSERT_OR_DIE(process_cb, "process callback is required");
    size_t entropy_len = word_count_to_bytes(word_count);
    ASSERT_OR_DIE(entropy_len <= MAX_ENTROPY_BYTES, "entropy length too large");

    uint8_t entropy[MAX_ENTROPY_BYTES];
    get_random(entropy, entropy_len, process_cb);
    char* words = NULL;
    ASSERT_OR_DIE(bip39_mnemonic_from_bytes(NULL, entropy, entropy_len, &words) == WALLY_OK,
                  "bip39_mnemonic_from_bytes failed");
    wally_bzero(entropy, sizeof(entropy));
    mnemonic_t* m = mnemonic_alloc(words, entropy_len);
    LOG_INFO("Mnemonic generated (%u words)", word_count);
    return m;
}

mnemonic_t* mnemonic_from_entropy(const uint8_t* bytes, size_t byte_len) {
    ASSERT_OR_DIE(entropy_len_valid(byte_len), "entropy must be 16 or 32 bytes");

    char* words = NULL;
    ASSERT_OR_DIE(bip39_mnemonic_from_bytes(NULL, bytes, byte_len, &words) == WALLY_OK,
                  "bip39_mnemonic_from_bytes failed");
    mnemonic_t* m = mnemonic_alloc(words, byte_len);
    LOG_INFO("Mnemonic from entropy (%zu bytes)", byte_len);
    return m;
}

size_t mnemonic_to_entropy(const mnemonic_t* m, uint8_t* out) {
    ASSERT_OR_DIE(m, "null mnemonic");
    ASSERT_OR_DIE(out, "null output buffer");

    size_t written = 0;
    ASSERT_OR_DIE(bip39_mnemonic_to_bytes(NULL, m->words, out, m->entropy_len, &written) == WALLY_OK,
                  "bip39_mnemonic_to_bytes failed");
    ASSERT_OR_DIE(written == m->entropy_len, "entropy size mismatch");
    return written;
}

mnemonic_t* mnemonic_combine(mnemonic_t* a, mnemonic_t* b) {
    ASSERT_OR_DIE(a && b, "null mnemonic");
    ASSERT_OR_DIE(a->entropy_len == b->entropy_len, "word count mismatch");

    uint8_t ea[MAX_ENTROPY_BYTES], eb[MAX_ENTROPY_BYTES];
    mnemonic_to_entropy(a, ea);
    mnemonic_to_entropy(b, eb);

    // XOR the entropy bytes together
    for (size_t i = 0; i < a->entropy_len; i++) {
        ea[i] ^= eb[i];
    }

    size_t entropy_len = a->entropy_len;  // save before discard

    mnemonic_discard(a);
    mnemonic_discard(b);

    mnemonic_t* result = mnemonic_from_entropy(ea, entropy_len);
    wally_bzero(ea, sizeof(ea));
    wally_bzero(eb, sizeof(eb));
    LOG_INFO("Mnemonics combined");
    return result;
}

mnemonic_t* mnemonic_from_string(const char* words) {
    if (!words || !*words) return NULL;

    /* Validate via libwally */
    size_t written = 0;
    uint8_t entropy[MAX_ENTROPY_BYTES];
    int rc = bip39_mnemonic_to_bytes(NULL, words, entropy, sizeof(entropy), &written);
    if (rc != WALLY_OK) { LOG_ERROR("Invalid mnemonic"); return NULL; }
    /* Copy the string, determine length from written bytes */
    char *copy = strdup(words);
    ASSERT_OR_DIE(copy, "out of memory");
    mnemonic_t* m = mnemonic_alloc(copy, written);
    LOG_INFO("Mnemonic from string (%zu-byte entropy)", written);
    return m;
}

void mnemonic_discard(mnemonic_t* m) {
    ASSERT_OR_DIE(m, "null mnemonic");
    if (m->words && m->stack) {
        secure_stack_pop(m->stack, (uint8_t*)m->words);
        free(m->words);
        LOG_INFO("mnemonic zeroed and freed");
    }
    if (m->stack) secure_stack_destroy(m->stack);
    free(m);
}
