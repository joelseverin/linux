/* SPDX-License-Identifier: GPL-2.0-only */

#include <linux/syscalls.h>
#include <asm/syscalls_wasm32.h>

SYSCALL_DEFINE3(wasm32_truncate64, const char __user *, pathname,
		sc_arg_u64_dual(length))
{
	return ksys_truncate(pathname, sc_arg_u64_glue(length));
}

SYSCALL_DEFINE3(wasm32_ftruncate64, unsigned int, fd, sc_arg_u64_dual(length))
{
	return ksys_ftruncate(fd, sc_arg_u64_glue(length));
}

SYSCALL_DEFINE5(wasm32_pread64, unsigned int, fd, char __user *, buf,
		size_t, count, sc_arg_u64_dual(pos))
{
	return ksys_pread64(fd, buf, count, sc_arg_u64_glue(pos));
}

SYSCALL_DEFINE5(wasm32_pwrite64, unsigned int, fd, const char __user *, buf,
		size_t, count, sc_arg_u64_dual(pos))
{
	return ksys_pwrite64(fd, buf, count, sc_arg_u64_glue(pos));
}

SYSCALL_DEFINE4(wasm32_readahead, int, fd, sc_arg_u64_dual(offset),
		size_t, count)
{
	return ksys_readahead(fd, sc_arg_u64_glue(offset), count);
}

SYSCALL_DEFINE6(wasm32_fadvise64_64, int, fd, sc_arg_u64_dual(offset),
		sc_arg_u64_dual(len), int, advice)
{
	return ksys_fadvise64_64(fd, sc_arg_u64_glue(offset),
				 sc_arg_u64_glue(len), advice);
}

SYSCALL_DEFINE6(wasm32_fallocate, int, fd, int, mode, sc_arg_u64_dual(offset),
		sc_arg_u64_dual(len))
{
	return ksys_fallocate(fd, mode, sc_arg_u64_glue(offset),
			      sc_arg_u64_glue(len));
}

SYSCALL_DEFINE6(wasm32_sync_file_range, int, fd, sc_arg_u64_dual(offset),
		sc_arg_u64_dual(nbytes), unsigned int, flags)
{
	return ksys_sync_file_range(fd, sc_arg_u64_glue(offset),
				    sc_arg_u64_glue(nbytes), flags);
}
