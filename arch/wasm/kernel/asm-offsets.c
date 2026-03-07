/* SPDX-License-Identifier: GPL-2.0-only */

#define COMPILE_OFFSETS

#include <linux/sched.h>
#include <linux/kbuild.h>
#include <asm/thread_info.h>

void asm_offsets(void);

void asm_offsets(void)
{
/*
 * struct task_struct is stored just above the thread stack. It is aligned by
 * L1_CACHE_BYTES, which is enforced by init_task and the task memory allocator.
 *
 * sizeof(pt_regs) and sizeof(task_struct) are naturally aligned by their size.
 * The start of the actual stack has to be 16-byte aligned when calling C code.
 */
#define _THREAD_TASK_STRUCT_OFFSET	ALIGN_DOWN(THREAD_SIZE - sizeof(struct task_struct), L1_CACHE_BYTES)
#define _THREAD_PT_REGS_OFFSET		(_THREAD_TASK_STRUCT_OFFSET - sizeof(struct pt_regs))
#define _THREAD_SWITCH_STACK_OFFSET	(_THREAD_PT_REGS_OFFSET - sizeof(struct switch_stack))
#define _THREAD_STACK_START		ALIGN_DOWN(_THREAD_SWITCH_STACK_OFFSET, 16)

	DEFINE(THREAD_TASK_STRUCT_OFFSET, _THREAD_TASK_STRUCT_OFFSET);
	BLANK();

	DEFINE(THREAD_PT_REGS_OFFSET, _THREAD_PT_REGS_OFFSET);
	OFFSET(PT_REGS_STACK_POINTER, pt_regs, stack_pointer);
	BLANK();

	DEFINE(THREAD_SWITCH_STACK_OFFSET, _THREAD_SWITCH_STACK_OFFSET);
	OFFSET(SWITCH_STACK_TLS, switch_stack, tls);
	BLANK();

	DEFINE(THREAD_STACK_START, _THREAD_STACK_START);
	BLANK();
}
