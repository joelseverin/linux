/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef _ASM_WASM_STACKTRACE_H
#define _ASM_WASM_STACKTRACE_H

#include <linux/sched.h>
#include <asm/ptrace.h>

#define WASM_STACKTRACE_MAX_SIZE 1000U

static inline bool on_thread_stack(void)
{
	/*
	 * Since current is directly derived from the stack pointer on Wasm, we
	 * can do this sneaky trick of comparing stack ends.
	 */
	return current->stack == (void*)((unsigned long)current & THREAD_MASK);
}

#endif /* _ASM_WASM_STACKTRACE_H */
