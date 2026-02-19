/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef _ASM_WASM_PTRACE_H
#define _ASM_WASM_PTRACE_H

#include <uapi/asm/ptrace.h>
#include <asm/cpuflags.h>
#include <asm/irqflags.h>

#ifndef __ASSEMBLER__

#define user_mode(regs) ((regs)->cpuflags & CPUFLAGS_USER_MODE)

/* Not available in Wasm. */
#define instruction_pointer(regs) (0)

#define current_user_stack_pointer() (0)

/* Not available (maybe we could extract this from a stacktrace?) */
#define profile_pc(regs) instruction_pointer(regs)

#define task_pt_regs(task) \
	((struct pt_regs *)(task_stack_page(task) + THREAD_SIZE) - 1U)
#define current_pt_regs() task_pt_regs(current)

#define task_switch_stack(task) ((struct switch_stack *)task_pt_regs(task) - 1U)
#define current_switch_stack() task_switch_stack(current)

static inline int regs_irqs_disabled(struct pt_regs *regs)
{
	return arch_irqs_disabled_flags(arch_local_save_flags());
}

#endif /* !__ASSEMBLER__ */

#endif /* _ASM_WASM_PTRACE_H */
