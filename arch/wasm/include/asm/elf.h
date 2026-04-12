/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef _ASM_WASM_ELF_H
#define _ASM_WASM_ELF_H

#include <uapi/asm/elf.h>

/*
 * ELF is not used with Wasm. These are just to comply with kernel dependencies.
 */
#ifndef ELF_CLASS
#ifdef CONFIG_64BIT
#define ELF_CLASS	ELFCLASS64
#else
#define ELF_CLASS	ELFCLASS32
#endif
#endif

#endif /* _ASM_WASM_ELF_H */
