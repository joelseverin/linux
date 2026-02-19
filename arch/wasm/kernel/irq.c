/* SPDX-License-Identifier: GPL-2.0-only */

#include <linux/irq.h>
#include <linux/irqchip.h>
#include <asm/smp.h>

static unsigned int wasm_irq_startup(struct irq_data *data)
{
	return 0;
}

static void wasm_irq_noop(struct irq_data *data)
{
}

static int
wasm_irq_set_affinity(struct irq_data *data, const struct cpumask *mask,
		     bool force)
{
#ifdef CONFIG_SMP
	printk("wasm_irq_set_affinity: %d %d %d", data->irq, cpumask_weight(mask), cpumask_first(mask));
	return 0;
#endif
}

struct irq_chip wasm_irq_chip = {
	.name			= "wasm",
	.irq_startup		= wasm_irq_startup,
	.irq_shutdown		= wasm_irq_noop,
	.irq_enable		= wasm_irq_noop,
	.irq_disable		= wasm_irq_noop,
	.irq_ack		= wasm_irq_noop,
	.irq_mask		= wasm_irq_noop,
	.irq_unmask		= wasm_irq_noop,
	.irq_set_affinity	= wasm_irq_set_affinity,
	.flags			= IRQCHIP_SKIP_SET_WAKE,
};

void __init init_IRQ(void)
{
	int irq;

	for (irq = 0; irq < NR_IRQS; ++irq) {
		if (irq == WASM_IRQ_IPI || irq == WASM_IRQ_TIMER) {
			irq_set_percpu_devid(irq);
			irq_set_chip_and_handler(
				irq, &wasm_irq_chip, handle_percpu_devid_irq);
		} else {
			irq_set_chip_and_handler(
				irq, &wasm_irq_chip, handle_simple_irq);
		}
	}

	setup_smp_ipi();
}
