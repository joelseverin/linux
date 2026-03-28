/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef _ASM_WASM_LINKAGE_H
#define _ASM_WASM_LINKAGE_H

#define cond_syscall(x) __asm__ (".weak " #x "\n\t"			\
				 ".type " #x ", @function\n\t"		\
				 ".set " #x ", sys_ni_syscall\n\t")

/* Not supported in Wasm. */
#define _THIS_IP_ (0)

#define ASM_NL \n

#endif /* _ASM_WASM_LINKAGE_H */
