/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef _ASM_WASM_DELAY_H
#define _ASM_WASM_DELAY_H

extern void __delay(unsigned long loops);
extern void __bad_udelay(void);
extern void __bad_ndelay(void);

/*
 * Wasm uses 1 loop = 1 nanosecond. This makes the conversion easy.
 *
 * Just like the rest of the kernel, these macros polices you if you try to
 * delay for too long. You should use a sleep function that calls schedule()
 * internally if you need longer sleeps than this. In Wasm in particular, usage
 * of these macros is really discouraged (what are you busy-waiting for?).
 */

#define udelay(n) (__builtin_constant_p(n) && (n) > 20000 ? \
			__bad_udelay() : __delay((n) * 1000))

#define ndelay(n) (__builtin_constant_p(n) && (n) > 20000000 ? \
			__bad_ndelay() : __delay(n))

#endif /* _ASM_WASM_DELAY_H */
