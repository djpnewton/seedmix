/**
 * @file main/crypto/secure_stack.h
 * @brief Guard stack for sensitive memory - enforces LIFO zeroing order.
 *
 * The stack stores *references* to sensitive buffers.  It does NOT own
 * or allocate memory.  On pop() the data is zeroed in place.  On destroy()
 * the stack asserts that all pushed items have been popped
 */

#ifndef SECURE_STACK_H
#define SECURE_STACK_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct secure_stack secure_stack_t;

secure_stack_t *secure_stack_create(size_t capacity);

/**
 * @brief Register a sensitive buffer.  The stack stores a reference only.
 */
bool secure_stack_push(secure_stack_t *stack, uint8_t *data, size_t len);

/**
 * @brief Zero the top buffer and remove it from the stack.  The caller
 *        must pass the exact same pointer that was pushed - if it doesn't
 *        match, this FATALs (catch bugs where the wrong buffer is popped).
 */
void secure_stack_pop(secure_stack_t *stack, uint8_t *data);

size_t secure_stack_count(const secure_stack_t *stack);

/**
 * @brief Destroy the stack.  FATALs if any items remain un-popped.
 */
void secure_stack_destroy(secure_stack_t *stack);

#ifdef __cplusplus
}
#endif

#endif /* SECURE_STACK_H */
