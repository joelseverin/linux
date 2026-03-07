/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef _ASM_WASM_CURRENT_H
#define _ASM_WASM_CURRENT_H

/*
 * Questionable but necessary to keep get_current() inline, due to the cyclic
 * dependency between task_struct and thread_info.
 */
#ifndef COMPILE_OFFSETS
#include <asm/asm-offsets.h>
#endif

#ifndef __ASSEMBLER__

#include <linux/linkage.h>
#include <asm/asm.h>
#include <asm/thread_info.h>

struct task_struct;

static __always_inline struct task_struct *get_current(void)
{
#ifndef COMPILE_OFFSETS
	unsigned long stack_pointer;

	__asm__ ("global.get __stack_pointer				\n\t"
		 "local.set %0						\n\t"
		 : "=r"(stack_pointer));

	return (struct task_struct *)((stack_pointer & THREAD_MASK)
				      + THREAD_TASK_STRUCT_OFFSET);
#else
	return NULL;
#endif
}

#define current (get_current())

#endif /* !__ASSEMBLER__ */

#endif /* _ASM_WASM_CURRENT_H */
