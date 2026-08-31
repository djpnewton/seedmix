/**
 * @file tests/support/fatal_test.h
 * @brief Helpers for asserting ASSERT_OR_DIE / FATAL paths.
 *
 * fatal_stub.c implements fatal_handler() so that, when armed, it longjmps
 * back into the calling test instead of exiting.  TEST_ASSERT_FATAL(expr)
 * asserts that `expr` triggers a FATAL; if it returns normally, the test
 * fails.
 */

#ifndef FATAL_TEST_H
#define FATAL_TEST_H

#include <setjmp.h>
#include <stdbool.h>

/** Arm the fatal handler: the next FATAL longjmps instead of exiting. */
void test_fatal_arm(void);

/** Disarm the fatal handler (restore exit-on-fatal behaviour). */
void test_fatal_disarm(void);

/** Jump buffer used by TEST_ASSERT_FATAL. */
jmp_buf* test_fatal_jmp_buf(void);

/**
 * Assert that `expr` triggers a FATAL (via ASSERT_OR_DIE / FATAL).
 * Fails the test if `expr` returns normally.
 */
#define TEST_ASSERT_FATAL(expr)                                                                    \
    do {                                                                                           \
        volatile int _fatal_caught = 0;                                                            \
        test_fatal_arm();                                                                          \
        if (setjmp(*test_fatal_jmp_buf()) == 0) {                                                  \
            (void)(expr);                                                                          \
        } else {                                                                                   \
            _fatal_caught = 1;                                                                     \
        }                                                                                          \
        test_fatal_disarm();                                                                       \
        TEST_ASSERT_TRUE_MESSAGE(_fatal_caught, "expected FATAL, but nothing happened");           \
    } while (0)

#endif /* FATAL_TEST_H */
