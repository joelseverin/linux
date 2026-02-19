/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef _ASM_WASM_PGTABLE_H
#define _ASM_WASM_PGTABLE_H

#include <asm-generic/pgtable-nopmd.h>

#include <linux/slab.h>
#include <asm/processor.h>
#include <asm/page.h>
#include <asm/io.h>

/*
 * No MMU support so do nothing...
 * Inspired by the various other NOMMU implementations in the kernel.
 */

#define pgd_present(pgd)	(1)
#define pgd_none(pgd)		(0)
#define pgd_bad(pgd)		(0)
#define pgd_clear(pgdp)
#define	pmd_offset(a, b)	((void *)0)

#define PAGE_NONE	__pgprot(0)
#define PAGE_SHARED	__pgprot(0)
#define PAGE_COPY	__pgprot(0)
#define PAGE_READONLY	__pgprot(0)
#define PAGE_KERNEL	__pgprot(0)

#define	VMALLOC_START	0
#define	VMALLOC_END	0xFFFFFFFF
#define	KMAP_START	0
#define	KMAP_END	0xFFFFFFFF

extern void paging_init(void);
#define swapper_pg_dir ((pgd_t *) 0)

/*
 * ZERO_PAGE is a global shared page that is always zero: used
 * for zero-mapped memory areas etc..
 */
extern unsigned long empty_zero_page[PAGE_SIZE / sizeof(unsigned long)];
#define ZERO_PAGE(vaddr) (virt_to_page(empty_zero_page))

#endif /* _ASM_WASM_PGTABLE_H */
