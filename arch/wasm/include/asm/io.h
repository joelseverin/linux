/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef _ASM_WASM_IO_H
#define _ASM_WASM_IO_H

/* asm-generic/io.h defines this as NULL which leads to compiler warnings. */
#define PCI_IOBASE ((void __iomem *)sizeof(long))

#include <asm-generic/io.h>

#endif /* _ASM_WASM_IO_H */
