/**
 * @file main/util/compiler.h
 * @brief Compiler annotations shared across the codebase.
 */

#ifndef COMPILER_H
#define COMPILER_H

#define NORETURN __attribute__((noreturn))
#define PRINTF_LIKE(fmt_idx, arg_idx) __attribute__((format(printf, fmt_idx, arg_idx)))

#endif /* COMPILER_H */
