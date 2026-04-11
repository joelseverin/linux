/* SPDX-License-Identifier: GPL-2.0-only */

#include <linux/random.h>
#include <linux/string.h>
#include <asm/random.h>

extern ssize_t wasm_random_get_bytes(u8* buffer, ssize_t count);

void __init setup_boot_rng(void)
{
	ssize_t size;
	u8 rng_seed[32];

	size = wasm_random_get_bytes(rng_seed, (ssize_t)sizeof(rng_seed));
	WARN_ON(size != (ssize_t)sizeof(rng_seed));

	if (size > 0) {
		add_bootloader_randomness(rng_seed, size);
		memzero_explicit(rng_seed, size);
	}
}
