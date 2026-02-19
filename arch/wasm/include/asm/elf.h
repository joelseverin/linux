/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef _ASM_WASM_ELF_H
#define _ASM_WASM_ELF_H

#include <uapi/linux/elf.h>
#include <linux/compat.h>
#include <uapi/asm/elf.h>
#include <asm/auxvec.h>
#include <asm/byteorder.h>

#define ELF_ARCH	EM_WASM

#ifndef ELF_CLASS
#ifdef CONFIG_64BIT
#define ELF_CLASS	ELFCLASS64
#else
#define ELF_CLASS	ELFCLASS32
#endif
#endif

#define ELF_DATA	ELFDATA2LSB

#define elf_check_arch(x) (((x)->e_machine == ELF_ARCH) && \
			   ((x)->e_ident[EI_CLASS] == ELF_CLASS))

extern bool compat_elf_check_arch(Elf32_Ehdr *hdr);
#define compat_elf_check_arch	compat_elf_check_arch

#define CORE_DUMP_USE_REGSET
#define ELF_EXEC_PAGESIZE	(PAGE_SIZE)

/*
 * This is the location that an ET_DYN program is loaded if exec'ed.  Typical
 * use of this is to invoke "./ld.so someprog" to test out a new version of
 * the loader.  We need to make sure that it is out of the way of the program
 * that it will "exec", and that there is sufficient room for the brk.
 */
#define ELF_ET_DYN_BASE		((TASK_SIZE / 3) * 2)

#ifdef CONFIG_64BIT
#ifdef CONFIG_COMPAT
#define STACK_RND_MASK		(test_thread_flag(TIF_32BIT) ? \
				 0x7ff >> (PAGE_SHIFT - 12) : \
				 0x3ffff >> (PAGE_SHIFT - 12))
#else
#define STACK_RND_MASK		(0x3ffff >> (PAGE_SHIFT - 12))
#endif
#endif

/*
 * Provides information on the availiable set of ISA extensions to userspace,
 * via a bitmap that coorespends to each single-letter ISA extension.  This is
 * essentially defunct, but will remain for compatibility with userspace.
 */
#define ELF_HWCAP	(elf_hwcap & ((1UL << RISCV_ISA_EXT_BASE) - 1))
extern unsigned long elf_hwcap;

/*
 * This yields a string that ld.so will use to load implementation
 * specific libraries for optimization.  This is more specific in
 * intent than poking at uname or /proc/cpuinfo.
 */
#define ELF_PLATFORM	(NULL)

#define COMPAT_ELF_PLATFORM	(NULL)

#define ELF_CORE_COPY_REGS(dest, regs)			\
do {							\
	*(struct user_regs_struct *)&(dest) =		\
		*(struct user_regs_struct *)regs;	\
} while (0);

#endif /* _ASM_WASM_ELF_H */
