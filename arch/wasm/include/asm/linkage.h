/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef _ASM_WASM_LINKAGE_H
#define _ASM_WASM_LINKAGE_H

/*
 * llvm-wasm crashes when generating the output file using the default
 * definition in linux/linking.h. In addition to this, wasm-ld does not like it
 * if two function signatures don't match, ruling out using a function with
 * __attribute__ ((weak, alias("sys_ni_syscall"))) here, even if it is lacking a
 * prototype (it assumes "one" (void) param).
 *
 * This has to be fixed by the host (or possibly some post-process build script)
 * because there is no way to tell which prototype to use for which symbol.
 * Getting rid of these stray declarations to begin with (e.g. setting
 * ARCH_HAS_SYSCALL_WRAPPER) unfortunately causes problems for the
 * sys_call_table generation. sys_call_table could be generated in some other
 * way (or shape) but that would require other hacks to find available syscalls.
 */
#define cond_syscall(x)

/* Not supported in Wasm. */
#define _THIS_IP_ (0)

#endif /* _ASM_WASM_LINKAGE_H */
