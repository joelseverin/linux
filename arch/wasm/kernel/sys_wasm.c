/* SPDX-License-Identifier: GPL-2.0-only */

#include <linux/syscalls.h>

// SYS_mmap2()
SYSCALL_DEFINE6(mmap2, unsigned long, addr, unsigned long, len,
	unsigned long, prot, unsigned long, flags, unsigned long, fd,
	unsigned long, pgoff)
{
	/*
	 * The "page size" for mmap2 should always be 4K (shift 12). Some
	 * architectures use their native page size or whatnot, and that's why
	 * this syscall exists in its own version for each architecture...
	 *
	 * Some architectures check the alignment, but that's out of spec.
	 */
	return ksys_mmap_pgoff(addr, len, prot, flags, fd,
			       pgoff >> (PAGE_SHIFT - 12));
}
