/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef _ASM_WASM_CPUFLAGS_H
#define _ASM_WASM_CPUFLAGS_H

#include <linux/percpu.h>

typedef unsigned int cpuflags_t;

/*
 * CPU flags handled by Wasm. These are used for accounting in many places.
 * CPUFLAGS_INTERRUPT: 1 if local interrupts are enabled.
 * CPUFLAGS_USER_MODE: 1 if we're not in privileged mode.
 */
#define CPUFLAGS_NONE		((cpuflags_t)0U)
#define CPUFLAGS_INTERRUPT	((cpuflags_t)BIT(0))
#define CPUFLAGS_USER_MODE	((cpuflags_t)BIT(1))

/*
 * These are values used where cpuflags are completely overwritten:
 * * Boot into privileged mode with interrupts disabled.
 * * Kthreads start in privileged mode with interrupts enabled.
 * * User-mode tasks start in protected mode with interrupts enabled.
 */
#define CPUFLAGS_BOOT_DEFAULT	    	(CPUFLAGS_NONE)
#define CPUFLAGS_KTHREAD_DEFAULT    	(CPUFLAGS_INTERRUPT)
#define CPUFLAGS_USER_TASK_DEFAULT	(CPUFLAGS_INTERRUPT | CPUFLAGS_USER_MODE)

/* Current per-cpu flags. Implementation may change - use accessors! */
DECLARE_PER_CPU(cpuflags_t, wasm_cpuflags);

static inline cpuflags_t cpuflags_load(void)
{
	return *this_cpu_ptr(&wasm_cpuflags);
}

static inline void cpuflags_overwrite(cpuflags_t mask)
{
	*this_cpu_ptr(&wasm_cpuflags) = mask;
}

static inline void cpuflags_clear_mask(cpuflags_t mask)
{
	*this_cpu_ptr(&wasm_cpuflags) &= ~mask;
}

static inline void cpuflags_set_mask(cpuflags_t mask)
{
	*this_cpu_ptr(&wasm_cpuflags) |= mask;
}

static inline bool cpuflags_test_any(cpuflags_t mask)
{
	return cpuflags_load() & mask;
}

static inline bool cpuflags_test_all(cpuflags_t mask)
{
	return (cpuflags_load() & mask) == mask;
}

#endif /* _ASM_WASM_CPUFLAGS_H */
