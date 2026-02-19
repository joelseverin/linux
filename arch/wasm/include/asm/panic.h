/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef _ASM_WASM_PANIC_H
#define _ASM_WASM_PANIC_H

#include <asm/wasm.h>

static inline void arch_panic_notify(const char *msg)
{
	wasm_panic(msg);
}

#include <asm-generic/panic.h>

#endif /* _ASM_WASM_PANIC_H */
