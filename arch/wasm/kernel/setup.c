/* SPDX-License-Identifier: GPL-2.0-only */

#include <linux/init.h>
#include <linux/screen_info.h>
#include <linux/memblock.h>
#include <linux/module.h>
#include <linux/mm.h>
#include <asm/processor.h>

/*
 * The format of "screen_info" is strange, and due to early
 * i386-setup code. This is just enough to make the console
 * code think we're on a VGA color display.
 */
struct screen_info screen_info = {
	.orig_x = 0,
	.orig_y = 25,
	.orig_video_cols = 80,
	.orig_video_lines = 25,
	.orig_video_isVGA = 1,
	.orig_video_points = 16,
};

unsigned long memory_start;
EXPORT_SYMBOL(memory_start);

unsigned long memory_end;
EXPORT_SYMBOL(memory_end);

unsigned long memory_kernel_break;
EXPORT_SYMBOL(memory_kernel_break);

__visible unsigned long wasm_memory_size(void)
{
	return __builtin_wasm_memory_size(0);
}

__visible unsigned long wasm_memory_grow(unsigned long delta_pages)
{
	return __builtin_wasm_memory_grow(0, delta_pages);
}

void __init smp_prepare_cpus(unsigned int max_cpus)
{
	unsigned i;

	for_each_possible_cpu(i)
		set_cpu_present(i, true);
}

static void smp_init_cpus(void)
{
	unsigned i;
	unsigned int ncpus = NR_CPUS; /* TODO: make this configurable */

	pr_info("%s: Core Count = %d\n", __func__, ncpus);

	if (ncpus > NR_CPUS) {
		ncpus = NR_CPUS;
		pr_info("%s: limiting core count by %d\n", __func__, ncpus);
	}

	for (i = 0; i < ncpus; ++i)
		set_cpu_possible(i, true);
}

void __init setup_arch(char **cmdline_p)
{
	unsigned long max_zone_pfn[MAX_NR_ZONES] = {0};

	/* Save unparsed command line copy for /proc/cmdline */
	*cmdline_p = boot_command_line;

	parse_early_param();

	/* See head.S for the logic that sets up these values. */
	memblock_reserve(memory_start, memory_kernel_break - memory_start);
	memblock_add(memory_start, memory_end - memory_start);

	/* pcpu_find_block_fit() returns signed 32-bit memory addresses, ugh. */
	memblock_set_current_limit(0x80000000); /* Only positive addresses. */

	/* This is needed so that more than 128 allocations can be made. */
	memblock_allow_resize();

	/* These are needed by some code to know which pages are valid. */
	max_pfn = PFN_DOWN(memory_end);
	min_low_pfn = PFN_DOWN(memory_start);
	max_low_pfn = max_pfn;

	/* Initialize zones, so that memory can be allocated beyond bootmem. */
	max_zone_pfn[ZONE_NORMAL] = memory_end >> PAGE_SHIFT;
	free_area_init(max_zone_pfn);

	smp_init_cpus();
}
