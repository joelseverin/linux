/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef _ASM_WASM_STRING_H
#define _ASM_WASM_STRING_H

#define __HAVE_ARCH_MEMSET
static inline void *memset(void *dst, int value, size_t size)
{
	/* memory.fill */
	__builtin_memset(dst, value, size);
	return dst;
}

#define __HAVE_ARCH_MEMCPY
static inline void *memcpy(void *dst, const void *src, size_t size)
{
	/* memory.copy */
	__builtin_memcpy(dst, src, size);
	return dst;
}

#define __HAVE_ARCH_MEMMOVE
static inline void *memmove(void *dst, const void *src, size_t size)
{
	/* memory.copy */
	__builtin_memmove(dst, src, size);
	return dst;
}

#endif /* _ASM_WASM_STRING_H */
