/* SPDX-License-Identifier: GPL-2.0-only */

#include <linux/clocksource.h>
#include <linux/clockchips.h>
#include <linux/interrupt.h>

#include <asm/irq.h>
#include <asm/smp.h>
#include <asm/time.h>

extern unsigned long long wasm_cpu_clock_get_monotonic(void);

/* Wasm clock source: derived from Wasm host cpu clock (monotonic). */

static unsigned long long wasm_clocksource_read(struct clocksource *cs)
{
	return wasm_cpu_clock_get_monotonic();
}

static struct clocksource wasm_clocksource = {
	.name = "wasm_cpu_clock",
	.flags = CLOCK_SOURCE_IS_CONTINUOUS,
	.rating = 200,
	.read = wasm_clocksource_read,
	.mask = CLOCKSOURCE_MASK(64),
};

static int __init wasm_clocksource_init(void)
{
	return clocksource_register_khz(&wasm_clocksource, 1000000U /* 1 ns */);
}

/* Wasm clock event: derived from Wasm atomic wait timeouts (in smp.c). */

static int wasm_clockevent_set_next_event(unsigned long delta,
	struct clock_event_device *dev)
{
	wasm_program_timer(delta);
	return 0;
}

static DEFINE_PER_CPU(struct clock_event_device, wasm_clockevents) = {
	.name			= "wasm_timer",
	.features		= CLOCK_EVT_FEAT_ONESHOT,
	.rating			= 300,
	.set_next_event		= wasm_clockevent_set_next_event,
};

static irqreturn_t __irq_entry wasm_timer_interrupt(int irq_nr, void *dev_id)
{
	struct clock_event_device *ce_dev = dev_id;

	ce_dev->event_handler(ce_dev);

	return IRQ_HANDLED;
}

static int __init wasm_clockevent_init(void)
{
	/* Requested here, enabled in wasm_clockevent_enable() for each cpu. */
	return request_percpu_irq(WASM_IRQ_TIMER, wasm_timer_interrupt,
				  "wasm-timer", &wasm_clockevents);
}

void wasm_clockevent_enable(void)
{
	struct clock_event_device *ce_dev = this_cpu_ptr(&wasm_clockevents);

	ce_dev->cpumask = cpumask_of(smp_processor_id());
	ce_dev->irq = WASM_IRQ_TIMER;
	clockevents_config_and_register(ce_dev, 1000000000, 0, ~0U);

	enable_percpu_irq(WASM_IRQ_TIMER, IRQ_TYPE_NONE);
}

/* Called very early in the boot, only CPU 0 is up so far! */
void __init time_init(void)
{
	/* Time is an illusion and yet here we are... */
	if (wasm_clocksource_init())
		panic("Failed to initialize Wasm clocksource");

	if (wasm_clockevent_init())
		panic("Failed to initialize Wasm clock_event");

	/* Only for CPU 0, secondaries will be enabled as they come up. */
	wasm_clockevent_enable();
}
