/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef _ASM_WASM_ENTRY_H
#define _ASM_WASM_ENTRY_H

__visible void start_secondary(void);

struct task_struct;

struct task_struct *
__switch_to(struct task_struct *prev_task, struct task_struct *next_task);

#define finish_arch_post_lock_switch finish_arch_post_lock_switch
static inline void finish_arch_post_lock_switch(void)
{
	if (raw_smp_processor_id() == IRQ_CPU)
		run_all_irqs();
}

asmlinkage unsigned
__ret_from_fork(struct task_struct *prev_task, struct task_struct *next_task);

__visible void raise_interrupt(int cpu, int irq_nr);

__visible void raise_exception(void);

#endif /* _ASM_WASM_ENTRY_H */
