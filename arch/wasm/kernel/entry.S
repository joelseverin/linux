/* SPDX-License-Identifier: GPL-2.0-only */

#include <asm/thread_info.h>

#include <asm/asm-offsets.h>


.globaltype __stack_pointer, i32

.section .globals,"",@

.globaltype __user_stack_pointer, i32
__user_stack_pointer:
.globaltype __user_tls_base, i32
__user_tls_base:

.section .text,"",@

.globl get_user_stack_pointer
get_user_stack_pointer:
	.functype get_user_stack_pointer() -> (i32)
	global.get __user_stack_pointer
	end_function

.globl get_user_tls_base
get_user_tls_base:
	.functype get_user_tls_base() -> (i32)
	global.get __user_tls_base
	end_function

.functype user_mode_tail() -> (i32)
.functype wasm_user_mode_tail(i32) -> ()

.globl _user_mode_tail
_user_mode_tail:
	.functype _user_mode_tail() -> ()
	.local i32 /* 0: flow */

	block
		call user_mode_tail
		local.tee 0
		i32.eqz
		br_if 0

		local.get 0
		call wasm_user_mode_tail
	end_block

	end_function

/*
 * HIGH ADDRESSES
 * --------------
 * (thread end)        <- (current & THREAD_MASK) + THREAD_SIZE
 * [task_struct]       <- current
 * [pt_regs]
 * [switch_stack]      <- initial __stack_pointer
 * (alignment)
 *                     <- Ready to call C code (16-byte aligned).
 * (...free space...)
 *
 * (thread start)      <- current & THREAD_MASK
 * -------------
 * LOW ADDRESSES
 */

.functype __ret_from_fork(i32, i32) -> (i32)

/* New process. Called by Wasm host when it runs a task for the first time. */
.globl ret_from_fork
ret_from_fork:
	/* struct task_struct *prev_task, struct task_struct *next_task */
	.functype ret_from_fork(i32, i32) -> (i32)

	/* We can't switch back to a task so no need to save into prev_task. */

	/* Load __stack_pointer from the new task's kernel stack area. */
	local.get 1
	i32.const zeroptr-THREAD_TASK_STRUCT_OFFSET+THREAD_STACK_START
	i32.add
	global.set __stack_pointer

	/*
	 * Finish up in C. Returns true if we have a clone callback to call.
	 * (Upon return, the correct cpuflags for userland have been loaded.)
	 */
	local.get 0
	local.get 1
	call __ret_from_fork

	/* Load __user_stack_pointer. */
	local.get 1
	i32.const zeroptr-THREAD_TASK_STRUCT_OFFSET+THREAD_PT_REGS_OFFSET+PT_REGS_STACK_POINTER
	i32.add
	i32.load 0
	global.set __user_stack_pointer

	/* Load __user_tls_base. */
	local.get 1
	i32.const zeroptr-THREAD_TASK_STRUCT_OFFSET+THREAD_SWITCH_STACK_OFFSET+SWITCH_STACK_TLS
	i32.add
	i32.load 0
	global.set __user_tls_base

	/* Clean up the stack. */
	global.get __stack_pointer
	i32.const zeroptr-THREAD_STACK_START+THREAD_TASK_STRUCT_OFFSET
	i32.add
	global.set __stack_pointer

	/* In theory, the first thing we execute may be a signal handler. */
	call _user_mode_tail

	end_function

.macro WASM_SYSCALL_ASM_HEAD
	/*
	 * The kernel expects pt_regs to be populated so save what we know.
	 * The following fields are saved in the C part of this handling:
	 * * cpuflags is set to appropriate values.
	 * * syscall_nr and syscall_args are set from our call parameters.
	 * * syscall_ret is set and returned to us.
	 */

	local.get 0
	global.set __user_stack_pointer
	local.get 1
	global.set __user_tls_base

	/* Allocate pt_regs + switch_stack + stack alignment. */
	global.get __stack_pointer
	i32.const zeroptr-THREAD_TASK_STRUCT_OFFSET+THREAD_STACK_START
	i32.add
	global.set __stack_pointer

	/* Save __user_stack_pointer. */
	global.get __stack_pointer
	i32.const zeroptr-THREAD_STACK_START+THREAD_PT_REGS_OFFSET+PT_REGS_STACK_POINTER
	i32.add
	global.get __user_stack_pointer
	i32.store 0

	/* Save __user_tls_base. */
	global.get __stack_pointer
	i32.const zeroptr-THREAD_STACK_START+THREAD_SWITCH_STACK_OFFSET+SWITCH_STACK_TLS
	i32.add
	global.get __user_tls_base
	i32.store 0

	/*
	 * Note: we don't need to swap the stack pointer, it already happened
	 * automatically when calling into the vmlinux Wasm instance.
	 */
.endm

.macro WASM_SYSCALL_ASM_FOOT
	/* (The cpuflags have already been restored in C.) */

	/* Load __user_stack_pointer. */
	global.get __stack_pointer
	i32.const zeroptr-THREAD_STACK_START+THREAD_PT_REGS_OFFSET+PT_REGS_STACK_POINTER
	i32.add
	i32.load 0
	global.set __user_stack_pointer

	/* Load __user_tls_base. */
	global.get __stack_pointer
	i32.const zeroptr-THREAD_STACK_START+THREAD_SWITCH_STACK_OFFSET+SWITCH_STACK_TLS
	i32.add
	i32.load 0
	global.set __user_tls_base

	/* Deallocate stack alignment + switch_stack + pt_regs. */
	global.get __stack_pointer
	i32.const zeroptr-THREAD_STACK_START+THREAD_TASK_STRUCT_OFFSET
	i32.add
	global.set __stack_pointer

	/*
	* Note: we don't need to swap the __stack_pointer, it will
	* happen automatically when returning back into the user code
	* Wasm instance (as that instance has its own __stack_pointer).
	*
	* We can exploit this as we're basically in userland, but with
	* the kernel stack pointer loaded. This allows us to play tricks
	* with execution in userland without setting a program counter.
	*
	* This is where signal handlers are called, and returned, and exec()
	* calls stop the execution of the user program. In the case of exec()
	* and signal return, the call stack collapses (this call never returns).
	*/
	call _user_mode_tail

	end_function
.endm

.functype __wasm_syscall_0(i32) -> (i32)
.functype __wasm_syscall_1(i32, i32) -> (i32)
.functype __wasm_syscall_2(i32, i32, i32) -> (i32)
.functype __wasm_syscall_3(i32, i32, i32, i32) -> (i32)
.functype __wasm_syscall_4(i32, i32, i32, i32, i32) -> (i32)
.functype __wasm_syscall_5(i32, i32, i32, i32, i32, i32) -> (i32)
.functype __wasm_syscall_6(i32, i32, i32, i32, i32, i32, i32) -> (i32)

/*
 * These syscall functions should be called from userland code. In order to skip
 * slow JavaScript glue code, they directly transfer all state needed into the
 * kernel. This means that two initial parammeters are added for sp and tp.
 * Parameter 0		[sp]: the userland stack pointer.
 * Parameter 1		[tp]: the userland TLS pointer.
 * Parameter 2		[nr]: the syscall nr.
 * Parameter 3..8	[argN]: syscall argument(s) 0..5, where applicable.
 *
 * The kernel never modifies sp or tp for the calling task during syscalls and
 * there is thus no need to restore them after the syscall returns. Apart from
 * diagnosics, they only play a role in the clone and exec family of syscalls.
 * Clone can be made to copy the supplied sp and tp to the new task. Exec should
 * maintain the tp even for new process images (this use case is quite sketchy).
 * Considering that the kernel does not care if userland even has a stack or TLS
 * area, it would be OK to not transfer these pointers at all if desired. In
 * both the clone and exec cases, the initial values of sp and tp would not be
 * loaded at a syscall site, but instead during the ret_from_fork code flow.
 */
.globl wasm_syscall_0
wasm_syscall_0:
	.functype wasm_syscall_0(i32, i32, i32) -> (i32)
	WASM_SYSCALL_ASM_HEAD
	local.get 2
	call __wasm_syscall_0
	WASM_SYSCALL_ASM_FOOT

.globl wasm_syscall_1
wasm_syscall_1:
	.functype wasm_syscall_1(i32, i32, i32, i32) -> (i32)
	WASM_SYSCALL_ASM_HEAD
	local.get 2
	local.get 3
	call __wasm_syscall_1
	WASM_SYSCALL_ASM_FOOT

.globl wasm_syscall_2
wasm_syscall_2:
	.functype wasm_syscall_2(i32, i32, i32, i32, i32) -> (i32)
	WASM_SYSCALL_ASM_HEAD
	local.get 2
	local.get 3
	local.get 4
	call __wasm_syscall_2
	WASM_SYSCALL_ASM_FOOT

.globl wasm_syscall_3
wasm_syscall_3:
	.functype wasm_syscall_3(i32, i32, i32, i32, i32, i32) -> (i32)
	WASM_SYSCALL_ASM_HEAD
	local.get 2
	local.get 3
	local.get 4
	local.get 5
	call __wasm_syscall_3
	WASM_SYSCALL_ASM_FOOT

.globl wasm_syscall_4
wasm_syscall_4:
	.functype wasm_syscall_4(i32, i32, i32, i32, i32, i32, i32) -> (i32)
	WASM_SYSCALL_ASM_HEAD
	local.get 2
	local.get 3
	local.get 4
	local.get 5
	local.get 6
	call __wasm_syscall_4
	WASM_SYSCALL_ASM_FOOT

.globl wasm_syscall_5
wasm_syscall_5:
	.functype wasm_syscall_5(i32, i32, i32, i32, i32, i32, i32, i32) -> (i32)
	WASM_SYSCALL_ASM_HEAD
	local.get 2
	local.get 3
	local.get 4
	local.get 5
	local.get 6
	local.get 7
	call __wasm_syscall_5
	WASM_SYSCALL_ASM_FOOT

.globl wasm_syscall_6
wasm_syscall_6:
	.functype wasm_syscall_6(i32, i32, i32, i32, i32, i32, i32, i32, i32) -> (i32)
	WASM_SYSCALL_ASM_HEAD
	local.get 2
	local.get 3
	local.get 4
	local.get 5
	local.get 6
	local.get 7
	local.get 8
	call __wasm_syscall_6
	WASM_SYSCALL_ASM_FOOT
