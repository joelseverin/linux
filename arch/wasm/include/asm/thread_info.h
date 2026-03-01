/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef _ASM_WASM_THREAD_INFO_H
#define _ASM_WASM_THREAD_INFO_H

#include <asm/page.h>
#include <asm/cache.h>

/*
 * In the Wasm arch, thread_info sits at the top of task_struct and both reside
 * at the very end of the stack area (which grows downwards).
 *
 * HIGHER ADDRESSES
 *
 * [ [ [...]        ] ] ^ <- (__stack_pointer & THREAD_MASK) + THREAD_SIZE
 * [ [ [thread_info]] ] |
 * [ [task_struct   ] ] | <- current, current_thread_info()
 * [ [stack         ] ] | <- (stack starts with pt_regs + possibly switch_stack)
 * [ [ [...]        ] ] | <- __stack_pointer (growing towards lower addresses)
 * [                  ] |
 * [    free space    ] | THREAD_SIZE
 * [                  ] v <- (__stack_pointer & THREAD_MASK)
 *
 * LOWER ADDRESSES
 *
 * As can be seen, current == current_thread_info() in this arch. In order to
 * access any of these, __stack_pointer can be masked by THREAD_MASK, since
 * the kernel stack for every task will be aligned on a THREAD_SIZE boundary.
 *
 * Example of memory-growing instructions	Resides in
 * --------------------------------------------	--------------------------------
 * iX.const, iX.load, local.get, global.get	Wasm internal stack
 * lobal.set __stack_pointer			__stack_pointer managed stack
 *
 * Stack usage in Wasm is pretty sparse. Most data resides in "locals" or on the
 * internal Wasm stack. Both of these are not accessible from within Wasm,
 * except outside the local usage of them of course. The stack we manage is used
 * for things that Wasm can't put on any of those, for exmaple when a pointer is
 * constructed when taking the address of an auto variable (i.e. the
 * function/block scope in C). That stack is referred to by the Wasm global
 * __stack_pointer and is known by the compiler. It is not part of the Wasm
 * standard, but makes certain parts of the C standard possible to compile. Two
 * pages should for this reason be enough as kernel stack. struct task_struct
 * (including struct thread_info at its base) is about 2K, leaving 6K for the
 * kernel stack.
 */
#define THREAD_SIZE_ORDER	(1)
#define THREAD_SIZE		(PAGE_SIZE << THREAD_SIZE_ORDER)
#define THREAD_MASK		(~(THREAD_SIZE - 1))

#ifndef __ASSEMBLY__

struct thread_info {
	unsigned int		cpu;
	int			preempt_count;	/* Needed but not really used */
	int			instance_depth;	/* 0 = idle task, 1 = running */
	unsigned long		syscall_work;	/* SYSCALL_WORK_ flags */

	/* The kernel plays tricks like casting to atomic_long_t* for flags. */
	unsigned long		flags __aligned(L1_CACHE_BYTES);
};

#define INIT_THREAD_INFO(tsk)			\
{						\
	.cpu = 0U,				\
	.flags = 0UL,				\
	.preempt_count = INIT_PREEMPT_COUNT,	\
	.instance_depth = 0,			\
}

struct task_struct;

static inline void *arch_alloc_thread_stack_node(
					struct task_struct *tsk, int node)
{
	return (void *)((unsigned long)tsk & THREAD_MASK);
}

static inline void arch_free_thread_stack(struct task_struct *tsk) { }

struct task_struct *alloc_task_struct_node(int node);
void free_task_struct(struct task_struct *tsk);

#endif /* !__ASSEMBLY__ */

#define TIF_SYSCALL_TRACE	0	/* syscall trace active */
#define TIF_SIGPENDING		1	/* signal pending */
#define TIF_NEED_RESCHED	2	/* rescheduling necessary */
#define TIF_NOTIFY_SIGNAL	3	/* signal notifications exist */
#define TIF_MEMDIE		4	/* is terminating due to OOM killer */
#define TIF_NOTIFY_RESUME	5	/* callback before returning to user */
#define TIF_NEVER_RUN		6	/* was never run by the scheduler */
#define TIF_RELOAD_PROGRAM	7	/* should reload code at syscall end */
#define TIF_DELIVER_SIGNAL	8	/* run sighandler at syscall end */
#define TIF_RETURN_SIGNAL	9	/* return sighandler at syscall end */
#define TIF_UPROBE		10	/* breakpointed or single-stepping */

#define _TIF_SYSCALL_TRACE	(1 << TIF_SYSCALL_TRACE)
#define _TIF_SIGPENDING		(1 << TIF_SIGPENDING)
#define _TIF_NEED_RESCHED	(1 << TIF_NEED_RESCHED)
#define _TIF_NOTIFY_SIGNAL	(1 << TIF_NOTIFY_SIGNAL)
#define _TIF_MEMDIE		(1 << TIF_MEMDIE)
#define _TIF_NOTIFY_RESUME	(1 << TIF_NOTIFY_RESUME)
#define _TIF_NEVER_RUN		(1 << TIF_NEVER_RUN)
#define _TIF_RELOAD_PROGRAM	(1 << TIF_RELOAD_PROGRAM)
#define _TIF_DELIVER_SIGNAL	(1 << TIF_DELIVER_SIGNAL)
#define _TIF_RETURN_SIGNAL	(1 << TIF_RETURN_SIGNAL)
#define _TIF_UPROBE		(1 << TIF_UPROBE)

#endif /* _ASM_WASM_THREAD_INFO_H */
