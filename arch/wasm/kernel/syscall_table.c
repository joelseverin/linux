/* SPDX-License-Identifier: GPL-2.0-only */

#include <linux/syscalls.h>
#include <asm-generic/syscalls.h>
#include <asm/syscall.h>

/*
 * We should probably use some soft variant of CONFIG_COMPAT yet to be invented.
 * TODO: This hack should be replaced with proper selection of compat syscalls!
 */
static long sys_truncate64_fixup(const char __user *pathname,
				  unsigned long length_lo,
				  unsigned long length_hi)
{
	return sys_truncate64(pathname,
		((unsigned long long)length_hi << 32) | length_lo);
}

static long sys_ftruncate64_fixup(unsigned int fd,
				  unsigned long length_lo,
				  unsigned long length_hi)
{
	return sys_ftruncate64(fd,
		((unsigned long long)length_hi << 32) | length_lo);
}

void (* const sys_call_table[__NR_syscalls])(void) = {
	[0 ... __NR_syscalls-1] = (void (*)(void))sys_ni_syscall,

#undef __SYSCALL
#define __SYSCALL(nr, call) [nr] = (void (*)(void))(call),
#include <asm/unistd.h>

	[__NR_truncate64] = (void (*)(void))sys_truncate64_fixup,
	[__NR_ftruncate64] = (void (*)(void))sys_ftruncate64_fixup,
};
