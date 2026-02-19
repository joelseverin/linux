/* SPDX-License-Identifier: GPL-2.0-only */

#include <linux/linkage.h>
#include <linux/init.h>
#include <linux/memblock.h>
#include <asm/page.h>

unsigned long empty_zero_page[PAGE_SIZE / sizeof(unsigned long)] __page_aligned_bss;
EXPORT_SYMBOL(empty_zero_page);

void __init mem_init(void)
{
	high_memory = (void *)memory_end;
}
