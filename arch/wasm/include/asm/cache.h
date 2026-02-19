/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef _ASM_WASM_CACHE_H
#define _ASM_WASM_CACHE_H

/*
 * Most architectures executing Wasm code have a cache line size of 64 bytes.
 */
#define L1_CACHE_SHIFT		(6)
#define L1_CACHE_BYTES		(1 << L1_CACHE_SHIFT)

#endif /* _ASM_WASM_CACHE_H */
