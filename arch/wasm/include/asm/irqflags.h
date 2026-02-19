/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef _ASM_WASM_IRQFLAGS_H
#define _ASM_WASM_IRQFLAGS_H

/*
 * arch_local_save_flags and arch_local_irq_restore are defined as non-static
 * functions as this header is included from places where percpu-variables and
 * even definitions for raw_smp_processor_id() cannot be included...
 */

#include <asm-generic/irqflags.h>

#endif /* _ASM_WASM_IRQFLAGS_H */
