/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef _ASM_WASM_CMPXCHG_H
#define _ASM_WASM_CMPXCHG_H

#include <linux/types.h>
#include <linux/irqflags.h>

/*
 * Inspired by:
 * https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2020/p0124r7.html
 * https://git.kernel.org/pub/scm/linux/kernel/git/dhowells/linux-fs.git/tree/include/asm-generic/iso-cmpxchg.h?h=iso-atomic
 *
 * TODO: McKenney et. al. above mention that atomic operations that return a
 * value should be marked with __ATOMIC_RELAXED and wrapped with
 * smp_mb__before_atomic()/smp_mb__after_atomic() calls. Howells above,
 * however, just applies __ATOMIC_SEQ_CST. What is the best approach?
 */

/*
 * This function doesn't exist, so you'll get a linker error if
 * something tries to do an invalidly-sized xchg().
 */
extern unsigned long long __generic_xchg_called_with_bad_pointer(void);

static __always_inline unsigned long long __generic_xchg(
	unsigned long long val, volatile void *ptr, int size)
{
	switch (size) {
	case 1:
		return __atomic_exchange_n(
			(volatile u8 *)ptr, (u8)val, __ATOMIC_SEQ_CST);

	case 2:
		return __atomic_exchange_n(
			(volatile u16 *)ptr, (u16)val, __ATOMIC_SEQ_CST);

	case 4:
		return __atomic_exchange_n(
			(volatile u32 *)ptr, (u32)val, __ATOMIC_SEQ_CST);

	case 8:
		return __atomic_exchange_n(
			(volatile u64 *)ptr, (u64)val, __ATOMIC_SEQ_CST);

	default:
		return __generic_xchg_called_with_bad_pointer();
	}
}

#define arch_xchg(ptr, x) ({							\
	((__typeof__(*(ptr))) __generic_xchg((unsigned long long)(x), (ptr),	\
			sizeof(*(ptr))));					\
})

static __always_inline unsigned long long __generic_cmpxchg(volatile void *ptr,
	unsigned long long oldVal, unsigned long long newVal, int size)
{
	/*
	 * Unlike this functions' signature, __atomic_compare_exchange_n will
	 * modify oldVal with the actual value if the compare fails.
	 */
	u8 expected8;
	u16 expected16;
	u32 expected32;
	u64 expected64;

	switch (size) {
	case 1:
		expected8 = (u8)oldVal;
		__atomic_compare_exchange_n(
			(volatile u8 *)ptr, &expected8, (u8)newVal,
			false, __ATOMIC_SEQ_CST, __ATOMIC_RELAXED);
		return expected8;

	case 2:
		expected16 = (u16)oldVal;
		__atomic_compare_exchange_n(
			(volatile u16 *)ptr, &expected16, (u16)newVal,
			false, __ATOMIC_SEQ_CST, __ATOMIC_RELAXED);
		return expected16;

	case 4:
		expected32 = (u32)oldVal;
		__atomic_compare_exchange_n(
			(volatile u32 *)ptr, &expected32, (u32)newVal,
			false, __ATOMIC_SEQ_CST, __ATOMIC_RELAXED);
		return expected32;

	case 8:
		expected64 = (u64)oldVal;
		__atomic_compare_exchange_n(
			(volatile u64 *)ptr, &expected64, (u64)newVal,
			false, __ATOMIC_SEQ_CST, __ATOMIC_RELAXED);
		return expected64;

	default:
		return __generic_xchg_called_with_bad_pointer();
	}
}

#define arch_cmpxchg(ptr, o, n) ({						\
	((__typeof__(*(ptr)))__generic_cmpxchg((ptr), (unsigned long long)(o),	\
			(unsigned long long)(n), sizeof(*(ptr))));			\
})

#define arch_cmpxchg64		arch_cmpxchg
#define arch_cmpxchg_local	arch_cmpxchg
#define arch_cmpxchg64_local	arch_cmpxchg

#endif /* _ASM_WASM_CMPXCHG_H */
