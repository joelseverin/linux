/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef _ASM_WASM_BARRIER_H
#define _ASM_WASM_BARRIER_H

/*
 * Inspired by:
 * https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2020/p0124r7.html
 */
#define __mb()	__atomic_thread_fence(__ATOMIC_SEQ_CST)
#define __rmb()	__atomic_thread_fence(__ATOMIC_ACQ_REL)
#define __wmb()	__atomic_thread_fence(__ATOMIC_ACQ_REL)

#include <asm-generic/barrier.h>

#endif /* _ASM_WASM_BARRIER_H */
