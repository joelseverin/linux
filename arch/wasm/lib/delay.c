/* SPDX-License-Identifier: GPL-2.0-only */

#include <linux/delay.h>

void calibrate_delay(void)
{
	/* Wasm convention: lpj = nanoseconds per Hz */
	loops_per_jiffy = 1000000000 / HZ;
}

void __delay(unsigned long cycles)
{
	unsigned int dummy = 0U;

	barrier();
	__builtin_wasm_memory_atomic_wait32(&dummy, 0U, (long long)cycles);
	barrier();
}
EXPORT_SYMBOL(__delay);
