/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef _ASM_WASM_FUTEX_H
#define _ASM_WASM_FUTEX_H

#include <linux/futex.h>
#include <linux/uaccess.h>
#include <asm/errno.h>

#define FUTEX_MAX_LOOPS	128

static inline int futex_atomic_cmpxchg_inatomic(u32 *uval, u32 __user *uaddr,
						u32 oldval, u32 newval)
{
	int loops;
	u32 expected;

	if (!access_ok(uaddr, sizeof(u32)))
		return -EFAULT;

	for (loops = 0; loops < FUTEX_MAX_LOOPS; ++loops) {
		expected = oldval;
		if (__atomic_compare_exchange_n((volatile u32 *)uaddr,
				&expected, newval, false, __ATOMIC_SEQ_CST,
				__ATOMIC_RELAXED)) {
			*uval = oldval;
			return 0;
		}
	}

	return -EAGAIN;
}

static inline int arch_futex_atomic_op_inuser(int op, u32 oparg, int *oval,
					      u32 __user *uaddr)
{
	if (!access_ok(uaddr, sizeof(u32)))
		return -EFAULT;

	switch (op) {
	case FUTEX_OP_SET:
		*oval = __atomic_exchange_n(
			(volatile u32 *)uaddr, oparg,  __ATOMIC_SEQ_CST);
		break;
	case FUTEX_OP_ADD:
		*oval = __atomic_fetch_add(
			(volatile u32 *)uaddr, oparg,  __ATOMIC_SEQ_CST);
		break;
	case FUTEX_OP_OR:
		*oval = __atomic_fetch_or(
			(volatile u32 *)uaddr, oparg,  __ATOMIC_SEQ_CST);
		break;
	case FUTEX_OP_ANDN:
		*oval = __atomic_fetch_and(
			(volatile u32 *)uaddr, ~oparg,  __ATOMIC_SEQ_CST);
		break;
	case FUTEX_OP_XOR:
		*oval = __atomic_fetch_xor(
			(volatile u32 *)uaddr, oparg,  __ATOMIC_SEQ_CST);
		break;
	default:
		return -ENOSYS;
	}

	return 0;
}

#endif /* _ASM_WASM_FUTEX_H */
