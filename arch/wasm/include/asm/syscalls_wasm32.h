/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef _ASM_WASM_SYSCALLS_WASM32_H
#define _ASM_WASM_SYSCALLS_WASM32_H

/* On wasm32, 64-bit parameters have to be passed in two 32-bit arguments. */
#ifndef CONFIG_64BIT

#define sc_arg_u64(name)	u32  name##_lo, u32  name##_hi
#define sc_arg_u64_dual(name)	u32, name##_lo, u32, name##_hi
#define sc_arg_u64_glue(name) \
	(((unsigned long long)name##_hi << 32) | (unsigned long long)name##_lo)

asmlinkage long sys_wasm32_truncate64(const char __user *pathname,
				      sc_arg_u64(length));

asmlinkage long sys_wasm32_ftruncate64(unsigned int fd, sc_arg_u64(length));

asmlinkage long sys_wasm32_pread64(unsigned int fd, char __user * buf,
				   size_t count, sc_arg_u64(pos));

asmlinkage long sys_wasm32_pwrite64(unsigned int fd, const char __user *buf,
				    size_t count, sc_arg_u64(pos));

asmlinkage long sys_wasm32_readahead(int fd, sc_arg_u64(offset), size_t count);

asmlinkage long sys_wasm32_fadvise64_64(int fd, sc_arg_u64(offset),
					sc_arg_u64(len), int advice);

asmlinkage long sys_wasm32_fallocate(int fd, int mode, sc_arg_u64(offset),
				     sc_arg_u64(len));

asmlinkage long sys_wasm32_sync_file_range(int fd, sc_arg_u64(offset),
					   sc_arg_u64(nbytes),
					   unsigned int flags);

#endif

#endif /* _ASM_WASM_SYSCALLS_WASM32_H */
