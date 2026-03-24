/* SPDX-License-Identifier: GPL-2.0-only */

#include <linux/clockchips.h>
#include <linux/completion.h>
#include <linux/smp.h>
#include <linux/cpu.h>
#include <linux/interrupt.h>
#include <linux/irq_work.h>
#include <linux/sched/task_stack.h>
#include <asm/entry.h>
#include <asm/time.h>
#include <asm/wasm.h>

extern unsigned long long wasm_cpu_clock_get_monotonic(void);

static DECLARE_COMPLETION(cpu_running);

#if NR_IRQS > 32
#error "NR_IRQS too high"
#endif
static DEFINE_PER_CPU(unsigned int, raised_irqs);

#define TIMER_NEVER_EXPIRE (-1)
static DEFINE_PER_CPU(long long, local_timer_expiries) = TIMER_NEVER_EXPIRE;

enum ipi_type {
	IPI_RESCHEDULE			= 0,
	IPI_CALL_FUNC			= 1,
	IPI_RECEIVE_BROADCAST		= 2,
	IPI_IRQ_WORK			= 3,
};
#define IPI_MASK(ipi_type) ((unsigned int)(1U << (int)(ipi_type)))
static DEFINE_PER_CPU(unsigned int, raised_ipis);

void smp_send_stop(void)
{
	unsigned int cpu;
	unsigned int this_cpu = smp_processor_id();

	for_each_online_cpu(cpu) {
		if (likely(cpu != this_cpu))
			wasm_stop_cpu(cpu);
	}
}

/* Run for each cpu except the first one, to bring the others up. */
int __cpu_up(unsigned int cpu, struct task_struct *idle_task)
{
	task_thread_info(idle_task)->cpu = cpu;

	/* Needed so that __switch_to does not create a new Wasm task. */
	task_thread_info(idle_task)->flags &= ~_TIF_NEVER_RUN;

	reinit_completion(&cpu_running);

	/* Will create a new Wasm instance and call _start_secondary(). */
	wasm_start_cpu(cpu, idle_task);

	/* Wait for CPU to finish startup & mark itself online before return. */
	wait_for_completion(&cpu_running);
	return 0;
}

/*
 * First thing to run on the secondary CPUs.
 *
 * Launched by __cpu_up(), which calls out to the Wasm host. The Wasm host calls
 * _start_secondary, which sets up the __stack_pointer and then calls us.
 */
__visible void start_secondary(void)
{
	unsigned int cpu = smp_processor_id();

	mmgrab(&init_mm);
	current->active_mm = &init_mm;
	BUG_ON(current->mm);

	notify_cpu_starting(cpu);
	set_cpu_online(cpu, true);

	enable_percpu_irq(WASM_IRQ_IPI, IRQ_TYPE_NONE);

	/*
	 * Notify boot CPU that we're up & online and it can safely return
	 * from __cpu_up(). IPIs need to be enabled (enable_percpu_irq above).
	 */
	complete(&cpu_running);

	wasm_clockevent_enable();

	local_irq_enable();
	cpu_startup_entry(CPUHP_AP_ONLINE_IDLE); /* Enter idle. */
}

void __init smp_cpus_done(unsigned int max_cpus)
{
	pr_info("SMP: Total of %d processors activated.\n", num_online_cpus());
}

__visible void raise_interrupt(int cpu, int irq_nr)
{
	/*
	 * Note: this function may be called independent of the kernel, outside
	 * any CPU or taks. Do not call kernel functions in here!
	 *
	 * per_cpu_ptr() is however safe to call (unlike e.g. this_cpu_ptr()).
	 */
	unsigned int *raised_irqs_ptr = per_cpu_ptr(&raised_irqs, cpu);

	if (irq_nr >= NR_IRQS)
		return;

	__atomic_or_fetch(raised_irqs_ptr, 1U << irq_nr, __ATOMIC_SEQ_CST);
	__builtin_wasm_memory_atomic_notify(raised_irqs_ptr, 1U);
}

static void send_ipi_message(int cpu, enum ipi_type ipi)
{
	unsigned int *raised_ipis_ptr = per_cpu_ptr(&raised_ipis, cpu);
	__atomic_or_fetch(raised_ipis_ptr, IPI_MASK(ipi), __ATOMIC_SEQ_CST);

	raise_interrupt(cpu, WASM_IRQ_IPI);
}

void arch_send_call_function_single_ipi(int cpu)
{
	/*
	 * This is not ideal, as these can only be handled when the other CPU is
	 * idle, but it allows using the kernel completion API when there is
	 * really only one task running. This is the case when the primary CPU
	 * is booting up secondaries, waiting for them to wake up and finish
	 * their part of the boot process. Ideally, we'd get rid of IPI calls.
	 */

	preempt_disable();
	send_ipi_message(cpu, IPI_CALL_FUNC);
	preempt_enable();
}

void arch_smp_send_reschedule(int cpu)
{
	preempt_disable();
	send_ipi_message(cpu, IPI_RESCHEDULE);
	preempt_enable();
}

extern void arch_irq_work_raise(void)
{
	/* This is a special IPI sent to ourselves, to break out of context. */

	preempt_disable();
	send_ipi_message(smp_processor_id(), IPI_IRQ_WORK);
	preempt_enable();
}

void tick_broadcast(const struct cpumask *mask)
{
	int cpu;

	preempt_disable();

	for_each_cpu(cpu, mask) {
		send_ipi_message(cpu, IPI_RECEIVE_BROADCAST);
	}

	preempt_enable();
}

void wasm_program_timer(unsigned long delta)
{
	unsigned long long now;
	unsigned long long expiry = 0ULL;

	unsigned int *raised_irqs_ptr = this_cpu_ptr(&raised_irqs);
	long long *expiry_ptr = this_cpu_ptr(&local_timer_expiries);

	if (delta == 0UL) {
		/* Optimization: set expiry to 0 to immediately expire. */
	} else {
		now = wasm_cpu_clock_get_monotonic();
		expiry = now + (unsigned long long)delta;

		/*
		 * This overflow will realistically never happen. Calling panic
		 * instead of returning a non-zero value is warranted, as the
		 * calling code would otherwise enter an infinite loop...
		 */
		if (expiry < now || expiry > (unsigned long long)LLONG_MAX)
			panic("clockevent expiry too large");
	}

	__atomic_store_n(expiry_ptr, (long long)expiry, __ATOMIC_SEQ_CST);

	/*
	 * We notify on raised_irqs since that's what we're waiting on in the
	 * idle loop. It does not matter if it's still 0 - it will wake anyway.
	 */
	__builtin_wasm_memory_atomic_notify(raised_irqs_ptr, 1U);
}

static irqreturn_t handle_IPI(int irq_nr, void *dev_id)
{
	unsigned int *ipi_mask_ptr = dev_id;
	unsigned int ipi_mask = __atomic_exchange_n(ipi_mask_ptr, 0U,
						    __ATOMIC_SEQ_CST);

	if (ipi_mask & IPI_MASK(IPI_RECEIVE_BROADCAST)) {
		/* Useful in NO_HZ_FULL case where no task is running. */
		tick_receive_broadcast();
	}

	if (ipi_mask & IPI_MASK(IPI_CALL_FUNC))
		generic_smp_call_function_interrupt();

	if (ipi_mask & IPI_MASK(IPI_RESCHEDULE))
		scheduler_ipi();

	if (ipi_mask & IPI_MASK(IPI_IRQ_WORK))
		irq_work_run();

	return IRQ_HANDLED;
}

void __init setup_smp_ipi(void)
{
	/* This is run on the boot cpu only. We need to enable others later. */

	if (request_percpu_irq(WASM_IRQ_IPI, handle_IPI, "IPI", &raised_ipis))
		panic("Failed to register IPI IRQ");

	enable_percpu_irq(WASM_IRQ_IPI, IRQ_TYPE_NONE);
}

void arch_cpu_idle(void)
{
	/* Note: The idle task will not migrate so per_cpu state is stable. */
	unsigned int *raised_irqs_ptr = this_cpu_ptr(&raised_irqs);
	unsigned int raised_irqs;
	long long *expiry_ptr = this_cpu_ptr(&local_timer_expiries);
	long long expiry;
	long long timeout;
	unsigned long long now;
	int irq_nr;

	/*
	 * This function is supposed to sleep until an interrupt comes in. The
	 * fact these events can only be detected from the idle task makes these
	 * "interrupts" unreliable unless there are no tasks on this CPU's
	 * runqueue at all times. Therefore, one CPU (IRQ_CPU) is dedicated to
	 * handle interrupts only, no user tasks are allowed to run on it.
	 *
	 * Additionally, the clockevent subsystem can wake us, either because it
	 * wants to program a new timer expiry (arming or re-arming the timer),
	 * or because an already armed timer is expiring. The clockevent
	 * subsystem can also request a broadcast - i.e. waking up other CPUs
	 * from a dedicated broadcast device (living on IRQ_CPU). It's important
	 * that all CPUs can handle programming of timers, since it's being used
	 * when the system boots (before NO_HZ_IDLE kicks in). Additionally,
	 * some kernel functions (e.g. schedule_timeout()) rely on timers to
	 * wake them up when no task is running on the CPU. These events and
	 * broadcasts will of course happen in a best-effort fashion on CPUs
	 * where there are tasks running, as they cannot be stopped.
	 *
	 * Wasm-specific wait primitives are used so that the Wasm VM can yield
	 * to the host OS. In a sense, it's like calling schedule(), but on the
	 * host. Callling schedule() here would just send us back, busy-waiting.
	 */
	for (;;) {
		expiry = __atomic_load_n(expiry_ptr, __ATOMIC_SEQ_CST);

reprocess:
		if (expiry > 0LL) {
			now = wasm_cpu_clock_get_monotonic();

			/* This will realistically never happen. */
			if (now > (unsigned long long)LLONG_MAX)
				panic("time is too far into the future");

			if ((long long)now >= expiry)
				timeout = 0LL;
			else
				timeout = expiry - now;
		} else {
			/*
			 * Just like magic:
			 * If expiry is 0 => timeout becomes 0.
			 * If expiry is forever => timeout becomes forever.
			 */
			timeout = expiry;
		}

		/* timeout == 0 iff the timer expired this iteration */
		if (timeout == 0LL) {
			/*
			 * It may be tempting to raise the timer interrupt
			 * already here, but that would not comply with the
			 * clockevent API, which mandates that re-programming
			 * of the device also cancels any pending event first.
			 */

			/* Try resetting the timer to never expire. */
			if (!__atomic_compare_exchange_n(expiry_ptr, &expiry,
					TIMER_NEVER_EXPIRE, false,
					__ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)) {
				/*
				 * Expiry changed under our rug - re-process it.
				 * This goto is slightly faster than "continue;"
				 * as the compare-and-swap above will already
				 * have loaded the new expiry value on failure.
				*/
				goto reprocess;
			}

			raise_interrupt(smp_processor_id(), WASM_IRQ_TIMER);

			if (smp_processor_id() != IRQ_CPU)
				timeout = TIMER_NEVER_EXPIRE;
		}

		if (timeout != 0LL)
			__builtin_wasm_memory_atomic_wait32(raised_irqs_ptr, 0U,
							    timeout);

		raised_irqs = __atomic_exchange_n(raised_irqs_ptr, 0U,
						  __ATOMIC_SEQ_CST);

		/*
		 * In the case of some raised_irqs, handle it, then we will come
		 * back here in a future invocation of this function. This
		 * function retuns so that that idle framework can do its job,
		 * for example if TIF_NEEDS_RESCHED is set by some IPI.
		 */
		if (raised_irqs)
			break;
	}

	irq_nr = 0;
	while (raised_irqs) {
		if (raised_irqs & 1U)
			do_irq_stacked(irq_nr);

		raised_irqs >>= 1;
		++irq_nr;
	}
}
