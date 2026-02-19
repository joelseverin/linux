/* SPDX-License-Identifier: GPL-2.0-only */

#include <linux/entry-common.h>
#include <linux/syscalls.h>
#include <asm/cpuflags.h>
#include <asm/entry.h>
#include <asm/processor.h>
#include <asm/ptrace.h>
#include <asm/syscall.h>

static inline void _exception_enter(struct pt_regs *regs)
{
	regs->cpuflags = cpuflags_load();
	cpuflags_clear_mask(CPUFLAGS_INTERRUPT | CPUFLAGS_USER_MODE);
}

static inline void _exception_exit(struct pt_regs *regs)
{
	cpuflags_overwrite(regs->cpuflags);
}

#define WASM_SYSCALL_N(x, args, cast_args, ...)					\
	__visible long __wasm_syscall_##x args;					\
	__visible long __wasm_syscall_##x args					\
	{									\
		long syscall = n;						\
		struct pt_regs *regs = current_pt_regs();			\
		void (*fn)(void);						\
		long syscall_args[] = {__MAP(x,__SC_ARGS,__VA_ARGS__)};		\
		bool restart;							\
										\
		_exception_enter(regs);						\
										\
		regs->syscall_nr = n;						\
		memcpy(regs->syscall_args, syscall_args, sizeof(syscall_args));	\
		regs->syscall_ret = -ENOSYS;					\
										\
		if (user_mode(regs)) {						\
			do {							\
				syscall = syscall_enter_from_user_mode(		\
					regs, syscall);				\
										\
				if (syscall >= 0 && syscall < __NR_syscalls) {	\
					fn = sys_call_table[syscall];		\
					if (syscall == __NR_restart_syscall) {	\
						regs->syscall_ret = sys_restart_syscall(); \
					} else if (fn != (void (*)(void))(void*)sys_ni_syscall) { \
						regs->syscall_ret = ((long (*)(cast_args))(void*)fn) \
							(__MAP(x,__SC_ARGS,__VA_ARGS__)); \
					}					\
				}						\
										\
				syscall_exit_to_user_mode(regs);		\
										\
				switch (regs->syscall_ret) {			\
				case -ERESTART_RESTARTBLOCK:			\
					syscall = __NR_restart_syscall;		\
					fallthrough;				\
				case -ERESTARTNOHAND:				\
				case -ERESTARTSYS:				\
				case -ERESTARTNOINTR:				\
					restart = true;				\
					break;					\
				default:					\
					restart = false;			\
				}						\
			} while (restart);					\
		} else {							\
			irqentry_state_t state = irqentry_nmi_enter(regs);	\
										\
			panic("Syscall called when in kernel mode");		\
										\
			irqentry_nmi_exit(regs, state);				\
		}								\
										\
		_exception_exit(regs);						\
										\
		return regs->syscall_ret;					\
	}
#define WASM_SYSCALL(x, ...) WASM_SYSCALL_N(					\
	x,									\
	(long n, __MAP(x,__SC_DECL,__VA_ARGS__)),				\
	__MAP(x,__SC_DECL,__VA_ARGS__),						\
	__VA_ARGS__)

WASM_SYSCALL_N(0, (long n), void)
WASM_SYSCALL(1, long, a)
WASM_SYSCALL(2, long, a, long, b)
WASM_SYSCALL(3, long, a, long, b, long, c)
WASM_SYSCALL(4, long, a, long, b, long, c, long, d)
WASM_SYSCALL(5, long, a, long, b, long, c, long, d, long, e)
WASM_SYSCALL(6, long, a, long, b, long, c, long, d, long, e, long, f)

/*
 * Final check before syscall return (after pt_regs have been restored).
 *
 * If exec() was called, we reload user program code. If there is a signal
 * handler to call, we call it. (Both will not happen, as exec blocks handlers.)
 *
 * Returns the direction of program flow:
 * -1 if exec() was called and the Wasm host should reload the user program.
 *  1 if a signal was delivered => the Wasm host should start signal handling.
 *  2 if a sigreturn happened => the Wasm host should cancel signal handling.
 *  3 if a signal was delivered AND a sigreturn (of an older signal handler)
 *    happened => the Wasm host should first handle the new signal (stacked),
 *    then cancel the old signal handler (after the stacked signal returns).
 *  0 if nothing should be done and the syscall should return normally.
 * In the case of exec(), the syscall should never fully return to the caller.
 */
int user_mode_tail(void)
{
	struct thread_info *thread_info = current_thread_info();
	const bool reload = thread_info->flags & _TIF_RELOAD_PROGRAM;
	const bool deliver = thread_info->flags & _TIF_DELIVER_SIGNAL;
	const bool retn = thread_info->flags & _TIF_RETURN_SIGNAL;

	if (reload) {
		BUG_ON(deliver);
		BUG_ON(retn);

		thread_info->flags &= ~_TIF_RELOAD_PROGRAM;
		return -1;
	} else if (deliver || retn) {
		BUG_ON(reload);

		if (deliver)
			thread_info->flags &= ~_TIF_DELIVER_SIGNAL;

		if (retn)
			thread_info->flags &= ~_TIF_RETURN_SIGNAL;

		return (deliver ? 1 : 0) | (retn ? 2 : 0);
	}

	return 0;
}

static void do_irq(struct pt_regs *regs, int irq_nr)
{
	struct pt_regs *old_regs;
	irqentry_state_t state = irqentry_enter(regs);

	irq_enter_rcu();
	old_regs = set_irq_regs(regs);
	generic_handle_irq(irq_nr);
	set_irq_regs(old_regs);
	irq_exit_rcu();

	irqentry_exit(regs, state);
}

void do_irq_stacked(int irq_nr)
{
	/*
	 * This is a bit odd but somewhere in this function's frame we start an
	 * exception frame. Exactly where the boundary is does not matter in
	 * practice, some data may end up on either "wrong" end of the boundary.
	 */
	struct pt_regs regs = PT_REGS_INIT;
	regs.stack_pointer = (unsigned long)&regs + sizeof(regs);
	_exception_enter(&regs);

	do_irq(&regs, irq_nr);

	_exception_exit(&regs);
}

/* Do an exception. There are currently no exception types in Wasm. */
static void do_exception(struct pt_regs *regs)
{
	/*
	 * The host is currently responsible for reporting the full error. We
	 * just mark this error as SIGILL but it could be anything.
	 */
	if (user_mode(regs)) {
		irqentry_enter_from_user_mode(regs);
		force_sig_fault(SIGILL, ILL_ILLOPC, (void __user *)0U);
		irqentry_exit_to_user_mode(regs);
	} else {
		irqentry_state_t state = irqentry_nmi_enter(regs);
		make_task_dead(SIGILL);
		irqentry_nmi_exit(regs, state);
	}
}

/*
* This function is called from the host when things break either in kernel code
* or user code. That code will never continue to execute - we have to report the
* error and try to recover in the best way possible.
*/
__visible void raise_exception(void)
{
	/*
	 * This is a bit odd but somewhere in this function's frame we start an
	 * exception frame. Exactly where the boundary is does not matter in
	 * practice, some data may end up on either "wrong" end of the boundary.
	 */
	struct pt_regs regs = PT_REGS_INIT;
	regs.stack_pointer = (unsigned long)&regs + sizeof(regs);
	_exception_enter(&regs);

	do_exception(&regs);

	_exception_exit(&regs);
}
