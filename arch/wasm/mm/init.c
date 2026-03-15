/* SPDX-License-Identifier: GPL-2.0-only */

#include <linux/linkage.h>
#include <linux/init.h>
#include <linux/memblock.h>
#include <asm/page.h>

unsigned long empty_zero_page[PAGE_SIZE / sizeof(unsigned long)] __page_aligned_bss;
EXPORT_SYMBOL(empty_zero_page);

void __init arch_zone_limits_init(unsigned long *max_zone_pfns)
{
	/* Initialize zones, so that memory can be allocated beyond bootmem. */
	max_zone_pfns[ZONE_NORMAL] = memory_end >> PAGE_SHIFT;
}

void __init mem_init(void)
{
	high_memory = (void *)memory_end;
}
