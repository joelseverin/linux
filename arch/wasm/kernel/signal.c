/* SPDX-License-Identifier: GPL-2.0-only */

#include <linux/entry-common.h>
#include <asm/ucontext.h>

struct rt_sigframe {
	int sig_param;

	/*
	 * info_param and uc_param are convenience pointers that allow extension
	 * of rt_sigframe in the future and easy detection of whether SA_SIGINFO
	 * is set (they are NULL if it is not set).
	 */
	siginfo_t *info_param;
	void *uc_param;

	union {
		/* Signature the kernel uses internaly ("opaque type"). */
		__sighandler_t sa_handler;

		/* Signature libc should use when SA_SIGINFO is set. */
		void (*sigaction)(int sig, siginfo_t *info, void *uc);

		/* Signature libc should use when SA_SIGINFO is not set. */
		void (*handler)(int sig);
	};

	/*
	 * info and uc are used for sigaction (SA_SIGINFO) but ignored for
	 * handler (!SA_SIGINFO). However, uc is still used by the kernel when
	 * handler is used, as it stores the regs to restore upon sigreturn.
	 * This is what allows signal handlers to be stacked.
	 */
	struct siginfo info;
	struct ucontext uc;
};

SYSCALL_DEFINE0(rt_sigreturn)
{
	struct pt_regs *regs = current_pt_regs();
	struct switch_stack *switch_stack = current_switch_stack();
	struct rt_sigframe __user *frame =
		(struct rt_sigframe __user *)regs->stack_pointer;
	struct user_regs_struct __user *user_regs = &frame->uc.uc_mcontext.regs;
	sigset_t set;

	/* Always make any pending restarted system calls return -EINTR */
	current->restart_block.fn = do_no_restart_syscall;

	if (!access_ok(frame, sizeof(*frame)))
		goto badframe;

	if (__copy_from_user(&set, &frame->uc.uc_sigmask, sizeof(set)))
		goto badframe;

	set_current_blocked(&set);

	if (__get_user(regs->stack_pointer, &user_regs->stack_pointer))
		goto badframe;
	if (__get_user(switch_stack->tls, &user_regs->tls))
		goto badframe;

	if (restore_altstack(&frame->uc.uc_stack))
		goto badframe;

	current_thread_info()->flags |= _TIF_RETURN_SIGNAL;

	return 0;

badframe:
	force_sig(SIGSEGV);

	return 0;
}

static int setup_rt_frame(struct ksignal *ksig, sigset_t *set,
	struct pt_regs *regs)
{
	struct switch_stack *switch_stack = (struct switch_stack *)regs - 1U;
	unsigned long stack_pointer = sigsp(regs->stack_pointer, ksig);
	struct rt_sigframe __user *frame;
	long err = 0;

	/*
	 * Allocate storage for frame, aligning it for itself and for further C
	 * function calling (which shoulld really be the largest alignment...).
	 */
	stack_pointer -= sizeof(*frame);
	stack_pointer = ALIGN_DOWN(stack_pointer, __alignof__(*frame));
	stack_pointer = ALIGN_DOWN(stack_pointer, STACK_ALIGN);
	frame = (struct rt_sigframe __user *)stack_pointer;

	if (!access_ok(frame, sizeof(*frame)))
		return -EFAULT;

	/* struct siginfo info */
	err |= copy_siginfo_to_user(&frame->info, &ksig->info);

	/* struct ucontext uc */
	err |= __put_user(0, &frame->uc.uc_flags);
	err |= __put_user(NULL, &frame->uc.uc_link);
	err |= __save_altstack(&frame->uc.uc_stack, regs->stack_pointer);
	err |= __put_user(regs->stack_pointer,
			  &frame->uc.uc_mcontext.regs.stack_pointer);
	err |= __put_user(switch_stack->tls, &frame->uc.uc_mcontext.regs.tls);
	err |= __copy_to_user(&frame->uc.uc_sigmask, set, sizeof(*set));

	/* int sig_param */
	err |= __put_user(ksig->sig, &frame->sig_param);

	/* info_param and uc_param convenience pointers */
	if (ksig->ka.sa.sa_flags & SA_SIGINFO) {
		err |= __put_user(&frame->info, &frame->info_param);
		err |= __put_user(&frame->uc, &frame->uc_param);
	} else {
		err |= __put_user(NULL, &frame->info_param);
		err |= __put_user(NULL, &frame->uc_param);
	}

	/* __sighandler_t sa_handler */
	err |= __put_user(ksig->ka.sa.sa_handler, &frame->sa_handler);

	if (err)
		return -EFAULT;

	regs->stack_pointer = stack_pointer;
	current_thread_info()->flags |= _TIF_DELIVER_SIGNAL;

	return 0;
}

static void handle_signal(struct ksignal *ksig, struct pt_regs *regs)
{
	sigset_t *oldset = sigmask_to_save();
	int ret;

	/* If we're from a syscall, cancel syscall restarting if appropriate. */
	if (regs->syscall_nr != -1) {
		switch (regs->syscall_ret) {
		case -ERESTART_RESTARTBLOCK:
		case -ERESTARTNOHAND:
			regs->syscall_ret = -EINTR;
			break;

		case -ERESTARTSYS:
			if (!(ksig->ka.sa.sa_flags & SA_RESTART)) {
				regs->syscall_ret = -EINTR;
				break;
			}
			fallthrough;
		case -ERESTARTNOINTR:
		default:
                        /* We will actually restart in these cases. */
			break;
		}
	}

	rseq_signal_deliver(ksig, regs);

	/*
	 * Wasm always uses rt-frames - the libc will have to figure out which
	 * signature to call the handler with depending on if SA_SIGINFO is set.
	 */
	ret = setup_rt_frame(ksig, oldset, regs);

	signal_setup_done(ret, ksig, 0);
}

void arch_do_signal_or_restart(struct pt_regs *regs)
{
	struct ksignal ksig;

	if (get_signal(&ksig)) {
		handle_signal(&ksig, regs);
		return;
	}

	/*
	 * Restart is handled in the syscall wrapper instead, as Wasm can't
	 * fiddle with the instruction pointer to re-run the syscall. (Restart
	 * may be canceled by handle_signal() above if we're handling a signal.)
	 */

	/*
	 * If there's no signal to deliver, we just put the saved sigmask
	 * back.
	 */
	restore_saved_sigmask();
}
