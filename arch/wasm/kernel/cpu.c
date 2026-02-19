/* SPDX-License-Identifier: GPL-2.0-only */

#include <linux/cpu.h>
#include <linux/seq_file.h>

#ifdef CONFIG_PROC_FS

static void *c_start(struct seq_file *m, loff_t *pos)
{
	if (*pos == nr_cpu_ids)
		return NULL;

	*pos = cpumask_next(*pos - 1, cpu_online_mask);
	if ((*pos) < nr_cpu_ids)
		return (void *)(uintptr_t)(1 + *pos);
	return NULL;
}

static void *c_next(struct seq_file *m, void *v, loff_t *pos)
{
	(*pos)++;
	return c_start(m, pos);
}

static void c_stop(struct seq_file *m, void *v)
{
}

static int c_show(struct seq_file *m, void *v)
{
	unsigned long cpu_id = (unsigned long)v - 1;

	seq_printf(m, "processor\t: %lu\n", cpu_id);
	seq_printf(m, "vendor_id\t: Wasm\n");

	return 0;
}

const struct seq_operations cpuinfo_op = {
	.start	= c_start,
	.next	= c_next,
	.stop	= c_stop,
	.show	= c_show,
};

#endif /* CONFIG_PROC_FS */
