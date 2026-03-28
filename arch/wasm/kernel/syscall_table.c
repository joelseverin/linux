/* SPDX-License-Identifier: GPL-2.0-only */

#include <linux/syscalls.h>
#include <asm-generic/syscalls.h>
#include <asm/syscalls_wasm32.h>
#include <asm/syscall.h>

void (* const sys_call_table[__NR_syscalls])(void) = {
	[0 ... __NR_syscalls-1] = (void (*)(void))(void*)sys_ni_syscall,

#undef __SYSCALL
#define __SYSCALL(nr, call) [nr] = (void (*)(void))(void*)(call),
#include <asm/unistd.h>

#ifndef CONFIG_64BIT
	/* Overwrite! */
	__SYSCALL(__NR_truncate64, sys_wasm32_truncate64)
	__SYSCALL(__NR_ftruncate64, sys_wasm32_ftruncate64)
	__SYSCALL(__NR_pread64, sys_wasm32_pread64)
	__SYSCALL(__NR_pwrite64, sys_wasm32_pwrite64)
	__SYSCALL(__NR_readahead, sys_wasm32_readahead)
	__SYSCALL(__NR_fadvise64_64, sys_wasm32_fadvise64_64)
	__SYSCALL(__NR_fallocate, sys_wasm32_fallocate)
	__SYSCALL(__NR_sync_file_range, sys_wasm32_sync_file_range)
#endif
};
