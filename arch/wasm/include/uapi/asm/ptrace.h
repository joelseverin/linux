/* SPDX-License-Identifier: GPL-2.0-only WITH Linux-syscall-note */

#ifndef _UAPI_ASM_WASM_PTRACE_H
#define _UAPI_ASM_WASM_PTRACE_H

#define PTRACE_SYSEMU			31
#define PTRACE_SYSEMU_SINGLESTEP	32

#ifndef __ASSEMBLY__

/* Registers stored during kernel entry (syscalls, IRQs and exceptions). */
struct pt_regs {
	unsigned long stack_pointer;	/* The __stack_pointer global. */
	unsigned long cpuflags;		/* CPU Flags (interrupt, user mode). */
	int syscall_nr;			/* Needed by syscall_get_nr() etc. */
	long syscall_args[6];		/* Needed by syscall_get_args() etc. */
	long syscall_ret;		/* Needed by syscall_*_return() etc. */
};

#define PT_REGS_INIT ((struct pt_regs){.syscall_nr = -1})

/* Registers stored when switching between user processes (and signals). */
struct switch_stack {
	/* When kthread, kernel thread callback with arg. */
	int (*fn)(void *);
	void *fn_arg;

	/*  When user task, the __tls_base global. Unused by the kernel. */
	unsigned long tls;
};

/* Registers for user processes (gdb etc.), stable ABI compared to pt_regs. */
struct user_regs_struct {
	unsigned long stack_pointer;
	unsigned long tls;
};

#endif /* __ASSEMBLY__ */
#endif /* _UAPI_ASM_WASM_PTRACE_H */
