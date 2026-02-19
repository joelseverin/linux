/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef _ASM_WASM_CURRENT_H
#define _ASM_WASM_CURRENT_H

/*
 * Questionable but necessary to keep get_current() inline, due to the cyclic
 * dependency between task_struct and thread_info.
 */
#ifndef ASM_OFFSETS_C
#include <asm/asm-offsets.h>
#endif

#ifndef __ASSEMBLY__

#include <linux/linkage.h>
#include <asm/thread_info.h>

struct task_struct;

static inline struct task_struct *get_current(void)
{
#ifndef ASM_OFFSETS_C
	char dummy; /* Something stored in the current kernel stack. */
	unsigned long thread_page = (unsigned long)&dummy & THREAD_MASK;
	return (struct task_struct *)(thread_page + THREAD_TASK_STRUCT_OFFSET);
#else
	return NULL;
#endif
}

#define current (get_current())

#endif /* !__ASSEMBLY__ */

#endif /* _ASM_WASM_CURRENT_H */
