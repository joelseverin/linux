/* SPDX-License-Identifier: GPL-2.0-only */

#include <linux/entry-common.h>
#include <linux/ptrace.h>
#include <linux/sched.h>
#include <linux/sched/debug.h>
#include <linux/sched/task_stack.h>
#include <linux/printk.h>
#include <asm/cpuflags.h>
#include <asm/entry.h>
#include <asm/wasm.h>

static cpumask_t user_cpus = CPU_MASK_NONE;

struct task_struct *__sched
__switch_to(struct task_struct *prev_task, struct task_struct *next_task)
{
	/*
	 * Here, a typical arch would normally:
	 * * Swap registers and stack.
	 * * Return to the instruction pointer of the new task.
	 *
	 * For new tasks (after fork), it would normally:
	 * * Swap registers and stack.
	 * * Call schedule_tail(), now in the context of the new process.
	 * * If there is a kernel_fn set, call it with correct callback arg.
	 * * Call syscall_exit_to_user_mode(current_pt_regs()) or equivalent.
	 *   -> Return to the new instruciton pointer, ending up in userland.
	 *
	 * When prev_task was swapped in again (on another reschedule), we would
	 * continue here and return back to the scheduler. However, Wasm can't
	 * do this. Only the in-memory part of the call stack can be swapped and
	 * there is no way to jump. Instead, we have to resort to serializing
	 * processes (in the cooperative multitasking sense) by launching
	 * several threads of execution on the host and use locks to make sure
	 * that only one process at a time is running on the same cpu.
	 *
	 * As soon as more CPUs are enabled, we can start running concurrently,
	 * by putting each task (except idle) on separate CPUs. Before that,
	 * init and kthreadd will need to both run, until smp is started.
	 * Thankfully we have control over these threads and know they will not
	 * hog the CPU. They might call schedule() on longer waits, that's fine.
	 *
	 * There is still the issue with idle threads, we could optimize to get
	 * away with them, which would cut the number of tasks used in the
	 * system by half. However, doing so is pretty annoying, as the idle
	 * loop is actually doing something and will eventually need to call
	 * schedule_idle(). For now, we serialize them too.
	 */

	struct task_struct *last_task;
	char name[TASK_COMM_LEN];

	/* For user code. */
	unsigned long bin_start = 0U;
	unsigned long bin_end = 0U;
	unsigned long data_start = 0U;

	if (task_thread_info(next_task)->flags & _TIF_NEVER_RUN) {
		task_thread_info(next_task)->flags &= ~_TIF_NEVER_RUN;

		/* Get the name to aid debugging. */
		get_task_comm(name, next_task);

		/* For user executables, we need to clone the Wasm instance. */
		if (next_task->mm->start_code) {
			bin_start = next_task->mm->start_code;
			bin_end = next_task->mm->end_code;
			data_start = next_task->mm->start_data;
		}

		/* This is called instead of serialize the first time. */
		last_task = wasm_create_and_run_task(prev_task, next_task, name,
			bin_start, bin_end, data_start, 0U);
	} else {
		last_task = wasm_serialize_tasks(prev_task, next_task);
	}

	/*
	 * If/when we reach here, we got __switch_to():ed by another task.
	 *
	 * All state we need to return to (__stack_pointer, current) is already
	 * part of this Wasm vmlinux instance and is thus implicitly swapped.
	 */

	/* last_task is the previous task (never prev_task, maybe next_task). */
	return last_task;
}

static int user_task_set_affinity(struct task_struct *p)
{
	/*
	 * TODO: This function needs a review of proper approach and locking!
	 * It's probably best to take a step back and think about how this
	 * should be implemented properly in the first place, instead of adding
	 * band aid on top of about every line that violates this and that. That
	 * includes fixing release_thread() and garbage collecting unused CPUs.
	 *
	 * We may also have to move kthreads to IRQ_CPU (with an option of the
	 * boot cpu before IRQ_CPU is up) in case they risk getting blocked.
	 */
	int retval;
	int cpu;

	/* Kthreads can be allowed to run on any online CPU. */
	if (p->flags & PF_KTHREAD)
		return 0;

hack:
	cpu = cpumask_first_zero(&user_cpus);
	if (cpu >= nr_cpu_ids)
		return -EBUSY;

	if(cpu == IRQ_CPU) {
		/* TODO: We should mark IRQ_CPU as taken at boot instead. */
		cpumask_set_cpu(cpu, &user_cpus);
		goto hack;
	}

	if (!cpu_online(cpu)) {
		BUG_ON(!cpu_possible(cpu));

		/* We should add_cpu(cpu) if we properly supported hotplug... */
		retval = cpu_device_up(get_cpu_device(cpu));
		if (retval)
			return retval;
	}

	cpumask_set_cpu(cpu, &user_cpus);

	retval = set_cpus_allowed_ptr(p, cpumask_of(cpu));
	if (retval) {
		cpumask_clear_cpu(cpu, &user_cpus);
		return retval;
	}

	p->flags |= PF_NO_SETAFFINITY;

	return 0;
}

asmlinkage unsigned
__ret_from_fork(struct task_struct *prev_task, struct task_struct *next_task)
{
	struct switch_stack *next_switch_stack = task_switch_stack(next_task);

	schedule_tail(prev_task);

	/* Kernel thread callback. */
	if (next_switch_stack->fn) {
		next_switch_stack->fn(next_switch_stack->fn_arg);
		/*
		* Kernel threads can return, and in doing so, return to user
		* space. This happens for the first user process (init).
		*/

		BUG_ON(current->flags & PF_KTHREAD);

		/*
		 * The binfmt loader would have set _TIF_RELOAD_PROGRAM
		 * but we clear it now so that future syscalls don't trap.
		 */
		current_thread_info()->flags &= ~_TIF_RELOAD_PROGRAM;
	}

	/*
	 * syscall_exit_to_user_mode() turns off interrupts, as most
	 * architectures would IRET right after it, enabling them again. We
	 * emulate this behaviour by loading cpuflags, which should both enable
	 * interrupts again but also drop the privilege level down to USER_MODE.
	 */
	syscall_exit_to_user_mode(current_pt_regs());
	cpuflags_overwrite(current_pt_regs()->cpuflags);

	/*
	 * After returning, the Wasm module binary will be initialized and run.
	 * We run any signal handlers that should be run first, then:
	 * kthread case: the host will call _start().
	 * clone callback case: the host will call __libc_clone_callback().
	 */
	return !(next_switch_stack->fn);
}

void flush_thread(void)
{
	/* Wasm has no FP state to reset, so do nothing. */
}

int copy_thread(struct task_struct *p, const struct kernel_clone_args *args)
{
	struct pt_regs *parent_pt_regs = current_pt_regs();
	struct switch_stack *parent_switch_stack = current_switch_stack();

	struct pt_regs *child_pt_regs = task_pt_regs(p);
	struct switch_stack *child_switch_stack = task_switch_stack(p);

	task_thread_info(p)->flags |= _TIF_NEVER_RUN;

	if (unlikely(args->fn)) {
		/* Kernel thread */
		memset(child_pt_regs, 0, sizeof(*child_pt_regs));
		child_pt_regs->cpuflags = CPUFLAGS_KTHREAD_DEFAULT;

		memset(child_switch_stack, 0, sizeof(*child_switch_stack));
		child_switch_stack->fn = args->fn;
		child_switch_stack->fn_arg = args->fn_arg;
	} else {
		/* User thread */
		*child_pt_regs = *parent_pt_regs;
		if (args->stack)
			child_pt_regs->stack_pointer = args->stack;

		*child_switch_stack = *parent_switch_stack;
		child_switch_stack->fn = NULL;
		child_switch_stack->fn_arg = NULL;
		if (args->flags & CLONE_SETTLS)
			child_switch_stack->tls = args->tls;
	}

	if (!p->mm->binfmt) {
		/* These are normally not zeroed out in copy_process(). */
		current->mm->start_code = 0;
		current->mm->end_code = 0;
		current->mm->start_stack = 0;
		current->mm->start_data = 0;
		current->mm->end_data = 0;
	}

	return user_task_set_affinity(p);
}

/*
 * Set up a thread for executing a new program.
 */
void start_thread(struct pt_regs *regs, unsigned long stack_pointer)
{
	memset(regs, 0, sizeof(*regs));
	regs->stack_pointer = stack_pointer;
	regs->cpuflags = CPUFLAGS_USER_TASK_DEFAULT;

	wasm_load_executable(current->mm->start_code, current->mm->end_code,
		current->mm->start_data, 0U);

	/* Reload the program when the current syscall exits. */
	current_thread_info()->flags |= _TIF_RELOAD_PROGRAM;
}

void release_thread(struct task_struct *dead_task)
{
	/* TODO: This code also needs review, like user_task_set_affinity(). */
	if (!(dead_task->flags & PF_KTHREAD)) {
		BUG_ON(dead_task->nr_cpus_allowed != 1);
		BUG_ON(cpumask_first(&dead_task->cpus_mask)
			!= task_thread_info(dead_task)->cpu);
		cpumask_clear_cpu(task_thread_info(dead_task)->cpu, &user_cpus);
	}

	wasm_release_task(dead_task);
}

void show_regs(struct pt_regs *regs)
{
	show_regs_print_info(KERN_DEFAULT);

	pr_cont("cpuflags: %08x sp: %08x flags: %08x preempt_count: %08x\n",
		(unsigned)regs->cpuflags,
		(unsigned)regs->stack_pointer,
		(unsigned)current_thread_info()->flags,
		(unsigned)current_thread_info()->preempt_count);
}

void show_stack(struct task_struct *task, unsigned long *stack,
		const char *loglvl)
{
	char *stack_trace;

	printk("%sStack from %08lx:", loglvl, (unsigned long)stack);

	stack_trace = kmalloc(WASM_STACKTRACE_MAX_SIZE, GFP_ATOMIC);
	if (stack_trace) {
		wasm_dump_stacktrace(stack_trace, WASM_STACKTRACE_MAX_SIZE);
		printk("%s", stack_trace);
	} else {
		printk("Failed to allocate stack trace buffer.");
	}
	kfree(stack_trace);
}
