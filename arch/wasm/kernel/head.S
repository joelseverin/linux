/* SPDX-License-Identifier: GPL-2.0-only */

#include <asm/processor.h>
#include <asm/asm.h>

#include <asm/asm-offsets.h>

.functype start_kernel() -> ()
.functype start_secondary() -> ()

/* vmlinux entrypoint */
.globl _start
_start:
	.functype _start() -> ()
	.local iLONG /* 0: alloc_limit */

	iLONG.const init_task
	global.set current

	/* This also implicitly aligns __stack_pointer for calling C code. */
	iLONG.const init_stack+CPU_THREAD_STACK_START
	global.set __stack_pointer

	/* Save static memory used by vmlinux. */
	iLONG.const memory_start
	iLONG.const CONFIG_KERNEL_START
	iLONG.store 0
	iLONG.const memory_kernel_break
	WASM_MEMORY_SIZE /* -> iLONG size */
	iLONG.const _WASM_PAGE_SIZE
	iLONG.mul
	iLONG.store 0

	iLONG.const zeroptr+CONFIG_BOOT_MEM_PAGES
	WASM_MEMORY_SIZE /* -> iLONG size */
	/* Try grow by the difference, (max - curr). */
	iLONG.sub
	local.set 0
	loop
		local.get 0
		WASM_MEMORY_GROW /* iLONG new -> iLONG old_or_error. */
		iLONG.const -1 /* -1 means allocation failed. */
		iLONG.eq
		br_if 0

		/*
		 * If we fail, keep decreasing. This often succeeds quite soon
		 * in browsers. The act of retrying allocations seem to work.
		 */
		local.get 0
		iLONG.const 1
		iLONG.sub
		local.set 0
	end_loop

	block
		local.get 0
		iLONG.const 16
		iLONG.lt_u
		br_if 0

		iLONG.const memory_end
		WASM_MEMORY_SIZE /* -> iLONG size */
		iLONG.const _WASM_PAGE_SIZE
		iLONG.mul
		iLONG.store 0

		call start_kernel /* Start the kernel! */
	end_block

	/* If we ever get here, the memory allocation failed. */
	end_function

.globl _start_secondary
_start_secondary:
	.functype _start_secondary(iLONG) -> ()

	/* current = idle_task */
	local.get 0
	global.set current

	/*
	 * __stack_pointer = task_stack_page(current) + CPU_THREAD_STACK_START
	 * where __stack_pointer becomes properly aligned for calling C code.
	 */
	global.get current
	iLONG.const zeroptr+TASK_STRUCT_STACK
	iLONG.add
	iLONG.load 0
	global.set __stack_pointer

	call start_secondary
	unreachable /* start_secondary should never return. */

	end_function
