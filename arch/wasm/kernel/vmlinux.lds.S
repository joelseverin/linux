/* SPDX-License-Identifier: GPL-2.0-only */

#include <asm/asm-offsets.h>

#include <asm/vmlinux.lds.h>
#include <asm/thread_info.h>
#include <asm/cache.h>
#include <asm/page.h>

/* Place init_task at the end of init_stack. */
#undef INIT_TASK_DATA
#define INIT_TASK_DATA(align)						\
	. = ALIGN(align);						\
	__start_init_stack = .;						\
	init_thread_union = .;						\
	init_stack = .;							\
	. = __start_init_stack + THREAD_TASK_STRUCT_OFFSET;		\
	KEEP(*(.data..init_task))					\
	KEEP(*(.data..init_thread_info))				\
	. = __start_init_stack + THREAD_SIZE;				\
	__end_init_stack = .;

/*
 * The rudimentary linker script support in wasm-ld does not permit a section to
 * be aligned directly. This workaround overcomes that limitation by immediately
 * aligning the dot at the start of each section instead. Otherwise, it's
 * functionally identical to the code in asm-generic/vmlinux.lds.h.
 */
#undef FW_LOADER_BUILT_IN_DATA
#define FW_LOADER_BUILT_IN_DATA						\
	. = ALIGN(8);							\
	.builtin_fw : AT(ADDR(.builtin_fw) - LOAD_OFFSET) {		\
		BOUNDED_SECTION_PRE_LABEL(.builtin_fw, _builtin_fw, __start, __end) \
	}

/*
 * Linker scripts in wasm-ld currently does not support alignment sorting.
 */
#define SORT_BY_ALIGNMENT(files) files

SECTIONS
{
	/* To refer to addres 0 in assembly, but as a relocation. */
	zeroptr = 0;

	/* Begin 1 Wasm page (65k) in so that we can dodge null-pointer. */
	. = 0x10000;

	__init_begin = .;
	HEAD_TEXT_SECTION
	INIT_TEXT_SECTION(PAGE_SIZE)
	INIT_DATA_SECTION(16)
	PERCPU_SECTION(L1_CACHE_BYTES)
	__init_end = .;

	.text : {
		_text = .;
		_stext = .;
		TEXT_TEXT
		SCHED_TEXT
		LOCK_TEXT
		KPROBES_TEXT
		ENTRY_TEXT
		IRQENTRY_TEXT
		SOFTIRQENTRY_TEXT
		_etext = .;
	}

	_sdata = .;
	RO_DATA(PAGE_SIZE)
	RW_DATA(L1_CACHE_BYTES, PAGE_SIZE, THREAD_SIZE)
	_edata = .;

	EXCEPTION_TABLE(16)

	BSS_SECTION(0, 0, 0)
	_end = .;

/*
	Not supported by wasm-ld linker script hack:
	STABS_DEBUG
	DWARF_DEBUG
	ELF_DETAILS
*/

	DISCARDS		// must be the last
}

/*
 * Due to the way linker scripts are implemented in wasm-ld, any symbol-alias
 * assignments have to happen after the symbol has been placed into the output.
 */
jiffies = jiffies_64;
