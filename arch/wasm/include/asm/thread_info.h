/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef _ASM_WASM_THREAD_INFO_H
#define _ASM_WASM_THREAD_INFO_H

#include <asm/page.h>
#include <asm/cache.h>

/*
 * Stack usage in Wasm is pretty sparse. Most data resides in "locals" or on the
 * internal Wasm stack. Both of these are not accessible from within Wasm,
 * except outside the local usage of them of course. The stack we manage is used
 * for things that Wasm can't put on any of those, for exmaple when a pointer is
 * constructed when taking the address of an auto variable (i.e. the
 * function/block scope in C). That stack is referred to by the Wasm global
 * __stack_pointer and is known by the compiler. It is not part of the Wasm
 * standard, but makes certain parts of the C standard possible to compile.
 *
 * Example of memory-growing instructions	Resides in
 * --------------------------------------------	--------------------------------
 * iX.const, iX.load, local.get, global.get	Wasm internal stack
 * lobal.set __stack_pointer			__stack_pointer managed stack
 *
 * One page should for this reason be enough for the kernel stack, even in the
 * case where one page is configured to be 4K.
 */
#define THREAD_SIZE_ORDER	(0)
#define THREAD_SIZE		(PAGE_SIZE << THREAD_SIZE_ORDER)
#define THREAD_MASK		(~(THREAD_SIZE - 1))

#ifndef __ASSEMBLER__

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

#endif /* !__ASSEMBLER__ */

/* Use the generic TIF infrastructure: */
#include <asm-generic/thread_info_tif.h>

/* Architecture specific bits (16-31): */
#define TIF_NEVER_RUN		16	/* was never run by the scheduler */
#define TIF_RELOAD_PROGRAM	17	/* should reload code at syscall end */
#define TIF_DELIVER_SIGNAL	18	/* run sighandler at syscall end */
#define TIF_RETURN_SIGNAL	19	/* return sighandler at syscall end */

#define _TIF_NEVER_RUN		(1 << TIF_NEVER_RUN)
#define _TIF_RELOAD_PROGRAM	(1 << TIF_RELOAD_PROGRAM)
#define _TIF_DELIVER_SIGNAL	(1 << TIF_DELIVER_SIGNAL)
#define _TIF_RETURN_SIGNAL	(1 << TIF_RETURN_SIGNAL)

#endif /* _ASM_WASM_THREAD_INFO_H */
