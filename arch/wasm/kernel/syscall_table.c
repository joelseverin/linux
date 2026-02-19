/* SPDX-License-Identifier: GPL-2.0-only */

#include <linux/syscalls.h>
#include <asm-generic/syscalls.h>
#include <asm/syscall.h>

#ifndef CONFIG_64BIT
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
#endif

void (* const sys_call_table[__NR_syscalls])(void) = {
	[0 ... __NR_syscalls-1] = (void (*)(void))(void*)sys_ni_syscall,

#undef __SYSCALL
#define __SYSCALL(nr, call) [nr] = (void (*)(void))(void*)(call),
#include <asm/unistd.h>

#ifndef CONFIG_64BIT
	/* Overwrite! */
	__SYSCALL(__NR_truncate64, sys_truncate64_fixup)
	__SYSCALL(__NR_ftruncate64, sys_ftruncate64_fixup)
#endif
};

/*
 * This is how we can tell the Wasm host what syscalls map to what numbers.
 *
 * Some syscalls numbers map to the same function (often ni_syscall) and in that
 * case we can't name the variable as the call due to name collision.
 */
#define SYSCALL_ENUMERATOR(nr, call) const char __syscall_nr_##call##_##nr;
#undef __SYSCALL
#define __SYSCALL(nr, call) SYSCALL_ENUMERATOR(nr, call)
#include <asm/unistd.h>
