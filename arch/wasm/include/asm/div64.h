/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef _ASM_WASM_DIV64_H
#define _ASM_WASM_DIV64_H

#include <linux/types.h>

/* From the 64-bit version in asm-generic/div64.h, used on wasm32 too. */
#define do_div(n, base)						\
({								\
	uint32_t __base = (base);				\
	uint32_t __rem;						\
	__rem = ((uint64_t)(n)) % __base;			\
	(n) = ((uint64_t)(n)) / __base;				\
	__rem;							\
})

/* Should never be called - prevent the generic prototype from being built. */
#define __div64_32

#if BITS_PER_LONG == 32

/* From the 64-bit versions in linux/math64.h, used on wasm32 too. */

static inline s64 div_s64_rem(s64 dividend, s32 divisor, s32 *remainder)
{
	*remainder = dividend % divisor;
	return dividend / divisor;
}
#define div_s64_rem div_s64_rem

static inline u64 div64_u64_rem(u64 dividend, u64 divisor, u64 *remainder)
{
	*remainder = dividend % divisor;
	return dividend / divisor;
}
#define div64_u64_rem div64_u64_rem

static inline u64 div64_u64(u64 dividend, u64 divisor)
{
	return dividend / divisor;
}
#define div64_u64 div64_u64

static inline s64 div64_s64(s64 dividend, s64 divisor)
{
	return dividend / divisor;
}
#define div64_s64 div64_s64

#endif

#endif /* _ASM_WASM_DIV64_H */
