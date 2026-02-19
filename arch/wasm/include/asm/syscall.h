/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef _ASM_WASM_SYSCALL_H
#define _ASM_WASM_SYSCALL_H

#include <uapi/linux/audit.h>
#include <asm/ptrace.h>

extern void (* const sys_call_table[])(void);

struct task_struct;

static inline long syscall_get_nr(struct task_struct *task,
				  struct pt_regs *regs)
{
	return regs->syscall_nr;
}

static inline void syscall_rollback(struct task_struct *task,
				    struct pt_regs *regs)
{
	/* We don't need to rollback anything on Wasm. */
}

static inline long syscall_get_error(struct task_struct *task,
				     struct pt_regs *regs)
{
	return IS_ERR_VALUE(regs->syscall_ret) ? regs->syscall_ret : 0;
}

static inline long syscall_get_return_value(struct task_struct *task,
					    struct pt_regs *regs)
{
	return regs->syscall_ret;
}

static inline void syscall_set_return_value(struct task_struct *task,
					    struct pt_regs *regs,
					    int error, long val)
{
	regs->syscall_ret = error ? (long)error : val;
}

static inline void syscall_get_arguments(struct task_struct *task,
					 struct pt_regs *regs,
					 unsigned long *args)
{
	args[0] = regs->syscall_nr;
	memcpy(&args[1], regs->syscall_args, sizeof(regs->syscall_args));
}

static inline int syscall_get_arch(struct task_struct *task)
{
#ifdef CONFIG_64BIT
	return AUDIT_ARCH_WASM64;
#else
	return AUDIT_ARCH_WASM32;
#endif
}

static inline bool arch_syscall_is_vdso_sigreturn(struct pt_regs *regs)
{
	return false;
}

#endif /* _ASM_WASM_SYSCALL_H */
