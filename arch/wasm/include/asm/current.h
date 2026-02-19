/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef _ASM_WASM_CURRENT_H
#define _ASM_WASM_CURRENT_H

#ifndef __ASSEMBLER__

#include <linux/linkage.h>
#include <asm/asm.h>
#include <asm/thread_info.h>

struct task_struct;

static __always_inline struct task_struct *get_current(void)
{
	struct task_struct *current_task;

	__asm__ ("global.get current					\n\t"
		 "local.set %0						\n\t"
		 : "=r"(current_task));

	return current_task;
}

#define current (get_current())

#endif /* !__ASSEMBLER__ */

#endif /* _ASM_WASM_CURRENT_H */
