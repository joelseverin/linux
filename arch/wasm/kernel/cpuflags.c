/* SPDX-License-Identifier: GPL-2.0-only */

#include <asm/cpuflags.h>

DEFINE_PER_CPU(cpuflags_t, wasm_cpuflags) = CPUFLAGS_BOOT_DEFAULT;
