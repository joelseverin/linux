/* SPDX-License-Identifier: GPL-2.0-only */

#include <linux/reboot.h>
#include <linux/irqflags.h>
#include <linux/smp.h>
#include <linux/printk.h>

static void machine_hang(void)
{
	unsigned int dummy = 0U;

	__builtin_wasm_memory_atomic_wait32(&dummy, 0U, -1LL);
}

void machine_restart(char *cmd)
{
	local_irq_disable();
	smp_send_stop();

	do_kernel_restart(cmd);

	printk("Reboot failed -- System halted\n");
	machine_hang();
}

void machine_halt(void)
{
	local_irq_disable();
	smp_send_stop();
	machine_hang();
}

void machine_power_off(void)
{
	local_irq_disable();
	smp_send_stop();
	do_kernel_power_off();
	machine_hang();
}
