/* SPDX-License-Identifier: GPL-2.0-only WITH Linux-syscall-note */

#ifndef _UAPI_ASM_WASM_ELF_H
#define _UAPI_ASM_WASM_ELF_H

#include <asm/ptrace.h>

/*
 * ELF is not used with Wasm. These are just to comply with kernel dependencies.
 */
typedef unsigned long elf_greg_t;
typedef struct user_regs_struct elf_gregset_t;
typedef unsigned long elf_fpregset_t;

#endif /* _UAPI_ASM_WASM_ELF_H */
