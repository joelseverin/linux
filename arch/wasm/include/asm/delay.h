/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Implementation based on asm-generic/delay.h. Wasm does not actually use loops
 * to delay and can simplify quite a bit by using 1 loop = 1 nanosecond. We can
 * thus also get rid of the __const_udelay complexity.
 */

#ifndef _ASM_WASM_DELAY_H
#define _ASM_WASM_DELAY_H

#include <vdso/time64.h>

extern void __delay(unsigned long loops);

/* Undefined function to get compile-time errors on too high constant delays. */
extern void __bad_delay(void);

/*
 * The maximum constant ndelay value picked out of thin air to prevent too long
 * constant ndelays (and udelays).
 */
					/* 20 ms */
#define DELAY_CONST_MAX_NDELAY		(20 * 1000 * 1000)

#define ndelay(n) (__builtin_constant_p(n) && (n) > DELAY_CONST_MAX_NDELAY ? \
			__bad_delay() : __delay(n))

#define udelay(n) ndelay((n) * NSEC_PER_USEC)

#endif /* _ASM_WASM_DELAY_H */
