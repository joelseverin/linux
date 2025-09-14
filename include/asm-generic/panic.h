/* SPDX-License-Identifier: GPL-2.0 */

#ifndef _ASM_GENERIC_PANIC_H
#define _ASM_GENERIC_PANIC_H

#ifndef CONFIG_ARCH_HAVE_PANIC_NOTIFY
static inline void arch_panic_notify(const char *msg)
{
}
#endif

#endif /* _ASM_GENERIC_PANIC_H */
