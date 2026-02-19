/* SPDX-License-Identifier: GPL-2.0-only */

#include <asm/irqflags.h>

#include <linux/bitops.h>
#include <asm/cpuflags.h>

unsigned long arch_local_save_flags(void)
{
	if (cpuflags_test_any(CPUFLAGS_INTERRUPT))
		return ARCH_IRQ_ENABLED;
	return ARCH_IRQ_DISABLED;
}

void arch_local_irq_restore(unsigned long flags)
{
	if (flags == ARCH_IRQ_DISABLED)
		cpuflags_clear_mask(CPUFLAGS_INTERRUPT);
	else
		cpuflags_set_mask(CPUFLAGS_INTERRUPT);
}
