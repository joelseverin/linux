/* SPDX-License-Identifier: GPL-2.0-only */

#include <linux/kasan.h>
#include <linux/preempt.h>
#include <linux/rcupdate.h>
#include <linux/spinlock.h>
#include <linux/gfp.h>
#include <linux/mm.h>

struct task_struct *alloc_task_struct_node(int node)
{
	struct page *page =
		alloc_pages_node(node, THREADINFO_GFP, THREAD_SIZE_ORDER);

	if (unlikely(!page))
		return NULL;

	return (struct task_struct *)((unsigned long)page_address(page) +
		ALIGN_DOWN(THREAD_SIZE - sizeof(struct task_struct),
			   L1_CACHE_BYTES));
}

void free_task_struct(struct task_struct *tsk)
{
	free_pages((unsigned long)tsk & THREAD_MASK, THREAD_SIZE_ORDER);
}
