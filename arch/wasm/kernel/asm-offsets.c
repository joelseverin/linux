/* SPDX-License-Identifier: GPL-2.0-only */

#define COMPILE_OFFSETS

#include <linux/sched.h>
#include <linux/kbuild.h>
#include <asm/thread_info.h>

void asm_offsets(void);

void asm_offsets(void)
{
/* Offset from task_stack_page(task) to task_pt_regs(task). */
#define _THREAD_PT_REGS_OFFSET ALIGN_DOWN( \
					THREAD_SIZE - sizeof(struct pt_regs), \
					__alignof__(struct pt_regs))

/* Offset from task_stack_page(task) to task_switch_stack(task). */
#define _THREAD_SWITCH_STACK_OFFSET ALIGN_DOWN( \
			_THREAD_PT_REGS_OFFSET - sizeof(struct switch_stack), \
			__alignof__(struct switch_stack))

/* The stack must be aligned when calling C code. */
#define _THREAD_STACK_START ALIGN_DOWN(_THREAD_SWITCH_STACK_OFFSET, STACK_ALIGN)

/* CPU stacks don't have switch_stack+pt_regs. (Alignment just for example.) */
#define _CPU_THREAD_STACK_START ALIGN_DOWN(THREAD_SIZE, STACK_ALIGN)

	DEFINE(_PAGE_SIZE, PAGE_SIZE);
	DEFINE(_WASM_PAGE_SIZE, WASM_PAGE_SIZE);
	BLANK();

	OFFSET(TASK_STRUCT_STACK, task_struct, stack);
	BLANK();

	DEFINE(THREAD_PT_REGS_OFFSET, _THREAD_PT_REGS_OFFSET);
	OFFSET(PT_REGS_STACK_POINTER, pt_regs, stack_pointer);
	BLANK();

	DEFINE(THREAD_SWITCH_STACK_OFFSET, _THREAD_SWITCH_STACK_OFFSET);
	OFFSET(SWITCH_STACK_TLS, switch_stack, tls);
	BLANK();

	DEFINE(THREAD_STACK_START, _THREAD_STACK_START);
	DEFINE(CPU_THREAD_STACK_START, _CPU_THREAD_STACK_START);
	BLANK();
}
