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
	iLONG.const 0x10000 /* The first page is reserved for trapping nullptr. */
	iLONG.store 0
	iLONG.const memory_kernel_break
	WASM_MEMORY_SIZE /* -> iLONG size */
	iLONG.const 0x10000 /* Multiply by Wasm page size (65k). */
	iLONG.mul
	iLONG.store 0

	/*
	 * By some trial-and-error in Firefox and (mostly) Chromium:
	 * * Allocating the full address space (4 GB) works most of the time.
	 * * Allocating 4 GB fails often enough to be unstable. Curiously, it
	 *   does not seem to have anything to do with free memory, and just
	 *   reloading the page fixes it. Waiting a bit might be beneficial.
	 * * Doing it from within Wasm seems to succeed more often(?).
	 * * Allocating it in one go works better than stepping, as below, and
	 *   would allow 4 GB. But stepping is more reliable.
	 * * Stepping all the way from 4 GB makes Chromium accept the allocation
	 *   soon enough, but then crash with SIGILL, probably because of OOM.
	 * * Stepping from 500 MB is a tradeoff with all things considered. It
	 *   ought indeed to be enough for anybody! (Oh, old joke, sorry...)
	 *
	 * Considering the above heuristics, a fair approach seems to start high
	 * and aggressively step downwards, one page at a time. But not too high
	 * or there will be OOM troubles related to current default browser
	 * settings. Sadly, browsers don't seem to handle this in a very well
	 * defined way, and we have to be moderately aggressive. An even more
	 * aggressive approach that surprisingly works is to try again and again
	 * with the same allocation size, but stepping almost achieves that.
	 *
	 * Whatever happens, the memory is zero-initialized and hopefully
	 * overcommitted by the host OS. If it is not, that should be fixed!
	 * Even better would be MMU support in Wasm, and this problem would be
	 * solved altogether. And a whole slew of other problems too!
	 *
	 * Note that we cannot allocate the last page from within Wasm (even
	 * though it is possible from the JavaScript host to create a Memory
	 * with initial: 0x10000, memory.grow only allows us to get to 0xFFFF).
	 * This is not too bad, as this is almost like not placing anything in
	 * the first page to catch null pointers. This guards underflow instead.
	 */
	iLONG.const 0x2000 /* Immediately decremented by 1 in the loop below. */
	WASM_MEMORY_SIZE /* -> iLONG size */
	iLONG.sub /* Try grow by the difference, (max - curr). */
	local.set 0
	loop
		local.get 0
		iLONG.const 1
		iLONG.sub
		local.tee 0

		WASM_MEMORY_GROW /* iLONG new -> iLONG old_or_error. */
		iLONG.const -1 /* -1 means allocation failed. */
		iLONG.eq
		br_if 0
	end_loop

	block
		local.get 0
		iLONG.const 16
		iLONG.lt_u
		br_if 0

		iLONG.const memory_end
		WASM_MEMORY_SIZE /* -> iLONG size */
		iLONG.const 0x10000 /* Multiply by Wasm page size (65k). */
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
