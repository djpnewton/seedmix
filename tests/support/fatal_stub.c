/**
 * @file tests/support/fatal_stub.c
 * @brief fatal_handler replacement for unit tests.
 *
 * By default a FATAL prints to stderr and exits.  When "armed" (see
 * fatal_test.h) it longjmps back into the calling test so ASSERT_OR_DIE /
 * FATAL paths can be asserted with TEST_ASSERT_FATAL().
 */

#include "fatal_test.h"
#include "util/error.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

static jmp_buf s_jmp;
static bool    s_armed = false;

void test_fatal_arm(void) { s_armed = true; }

void test_fatal_disarm(void) { s_armed = false; }

jmp_buf* test_fatal_jmp_buf(void) { return &s_jmp; }

NORETURN void fatal_handler(const char* file, int line, const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    if (s_armed) {
        va_end(ap);
        longjmp(s_jmp, 1);
    }

    fprintf(stderr, "FATAL %s:%d: ", file, line);
    vfprintf(stderr, fmt, ap);
    fputc('\n', stderr);
    va_end(ap);
    exit(1);
}
