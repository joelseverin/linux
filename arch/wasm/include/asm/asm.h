/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef _ASM_WASM_ASM_H
#define _ASM_WASM_ASM_H

#include <linux/stringify.h>
#include <asm/linkage.h>

#if __SIZEOF_POINTER__ == 8
#ifdef __ASSEMBLER__
#define iLONG				i64
#define INSN_32_64(insn32, insn64)	insn64
#else
#define iLONG				"i64"
#endif
#elif __SIZEOF_POINTER__ == 4
#ifdef __ASSEMBLER__
#define iLONG				i32
#define INSN_32_64(insn32, insn64)	insn32
#else
#define iLONG				"i32"
#endif
#else
#error "Unexpected __SIZEOF_POINTER__"
#endif

#ifdef __ASSEMBLER__
#define __ASM_DECL(name, type) .globaltype name, type
#else
#define __ASM_DECL(name, type) __asm__ (".globaltype " #name ", " type "\n\t");
#endif

__ASM_DECL(__stack_pointer, iLONG)
__ASM_DECL(current, iLONG)

/* Workaround for old LLVM that does not fully support polymorphic memory.*. */
#ifdef __ASSEMBLER__
.functype wasm_memory_size() -> (iLONG)
.functype wasm_memory_grow(iLONG) -> (iLONG)
#define WASM_MEMORY_SIZE	INSN_32_64(memory.size 0, call wasm_memory_size)
#define WASM_MEMORY_GROW	INSN_32_64(memory.grow 0, call wasm_memory_grow)
#endif

#endif /* _ASM_WASM_ASM_H */
