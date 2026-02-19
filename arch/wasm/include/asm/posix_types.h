/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef _ASM_WASM_POSIX_TYPES_H
#define _ASM_WASM_POSIX_TYPES_H

/**
 * Clang defines size_t/ssize_t to unsigned long/long on wasm32 which causes
 * -Wformat errors for %zu/%zd if size_t/ssize_t is typedef:ed unsigned int/int.
 */
typedef unsigned long   __kernel_size_t;
typedef long            __kernel_ssize_t;
typedef long	__kernel_ptrdiff_t;
#define __kernel_size_t __kernel_size_t

#include <asm-generic/posix_types.h>

#endif /* _ASM_WASM_POSIX_TYPES_H */
