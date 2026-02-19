/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef _ASM_WASM_ENTRY_COMMON_H
#define _ASM_WASM_ENTRY_COMMON_H

#include <asm/stacktrace.h>

/*
 * Needed by common/entry.c. Returning -1 signals failure, should it ever run...
 *
 * Wasm could in theory support seccomp, but the transformation from non-seccomp
 * to seccomp mode would require quite a bit of thought to get everything right.
 */
#define __secure_computing(...) (-1)

#endif /* _ASM_WASM_ENTRY_COMMON_H */
