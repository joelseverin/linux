/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef _ASM_WASM_WASM_H
#define _ASM_WASM_WASM_H

/* These are symbols imported from the Wasm host. */

extern void wasm_panic(const char *msg);
extern void wasm_dump_stacktrace(char* buffer, unsigned long max_size);

extern void wasm_start_cpu(unsigned int cpu, struct task_struct *idle_task,
	unsigned long start_stack);
extern void wasm_stop_cpu(unsigned int cpu);

extern struct task_struct *wasm_create_and_run_task(
	struct task_struct *prev_task, struct task_struct *new_task,
	const char *name, unsigned long bin_start, unsigned long bin_end,
	unsigned long data_start, unsigned long table_start);
extern void wasm_release_task(struct task_struct *dead_task);
extern struct task_struct *wasm_serialize_tasks(struct task_struct *prev_task,
	struct task_struct *next_task);

extern void wasm_load_executable(unsigned long bin_start, unsigned long bin_end,
	unsigned long data_start, unsigned long table_start);
extern void wasm_reload_program(void);

extern void wasm_clone_callback(void);

#endif /* _ASM_WASM_WASM_H */
