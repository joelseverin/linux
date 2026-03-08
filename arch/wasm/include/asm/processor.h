/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef _ASM_WASM_PROCESSOR_H
#define _ASM_WASM_PROCESSOR_H

/* Calling C code requires the stack to be 16-byte alignment (LLVM mandated). */
#define STACK_ALIGN 16

#ifndef __ASSEMBLER__

struct pt_regs;

/* 3 GB RAM for userspace, 1 GB for the kernel. */
#define TASK_SIZE (0xC0000000)

/*
 * We run interrupts on CPU 1, keep it clear. Why not CPU 0? Because init needs
 * to run on CPU 0 for a while. We don't need interrupts until SMP has started,
 * but we need init before.
 */
#define IRQ_CPU 1

#define cpu_relax()	barrier()

struct thread_struct {
};

#define INIT_THREAD { \
}

void start_thread(struct pt_regs *regs, unsigned long stack_pointer);

void do_irq_stacked(int irq_nr);

int user_mode_tail(void);

struct task_struct;
static inline unsigned long __get_wchan(struct task_struct *p)
{
    /* Should return the function before schedule() was called. */
    /* Will be shown under the "Waiting Channel" of the ps command. */
    return 0;
}

/* We don't have an instruction pointer. See instruction_pointer.h */
#define KSTK_EIP(task) (0)

/* We could possibly expose the stack pointer (has some data)...? */
#define KSTK_ESP(task) (0)

#endif /* __ASSEMBLER__ */

#endif /* _ASM_WASM_PROCESSOR_H */
