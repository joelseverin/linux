/* SPDX-License-Identifier: GPL-2.0-only */

#include <linux/irq.h>
#include <linux/irqchip.h>
#include <asm/smp.h>

static void wasm_irq_noop(struct irq_data *data)
{
}

struct irq_chip wasm_irq_chip = {
	.name			= "wasm",
	.irq_mask		= wasm_irq_noop,
	.irq_unmask		= wasm_irq_noop,
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
