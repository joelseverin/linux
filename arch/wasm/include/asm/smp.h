/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef _ASM_WASM_SMP_H
#define _ASM_WASM_SMP_H

#ifdef CONFIG_SMP

#define raw_smp_processor_id() (current_thread_info()->cpu)

void __init setup_smp_ipi(void);

void arch_send_call_function_single_ipi(int cpu);

static inline void arch_send_call_function_ipi_mask(const struct cpumask *mask)
{
	int cpu;

	for_each_cpu(cpu, mask)
		arch_send_call_function_single_ipi(cpu);
}

#endif /* !CONFIG_SMP */

#endif /* _ASM_WASM_SMP_H */
