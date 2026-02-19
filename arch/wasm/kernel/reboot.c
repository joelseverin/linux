/* SPDX-License-Identifier: GPL-2.0-only */

#include <linux/reboot.h>
#include <linux/irqflags.h>
#include <linux/smp.h>
#include <linux/printk.h>

void machine_restart(char *cmd)
{
	local_irq_disable();
	smp_send_stop();

	do_kernel_restart(cmd);

	printk("Reboot failed -- System halted\n");
	for (;;);
}

void machine_halt(void)
{
	local_irq_disable();
	smp_send_stop();
	for (;;);
}

void machine_power_off(void)
{
	local_irq_disable();
	smp_send_stop();
	do_kernel_power_off();
}
