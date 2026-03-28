/* SPDX-License-Identifier: GPL-2.0-only WITH Linux-syscall-note */

#include <asm/bitsperlong.h>

#define __ARCH_WANT_SYS_CLONE
#define __ARCH_WANT_SET_GET_RLIMIT

#if __BITS_PER_LONG == 64
#define __ARCH_WANT_NEW_STAT
#else
#define __ARCH_WANT_STAT64
#endif

#include <asm-generic/unistd.h>

#define NR_syscalls	__NR_syscalls
