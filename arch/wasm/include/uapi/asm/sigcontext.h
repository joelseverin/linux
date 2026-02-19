/* SPDX-License-Identifier: GPL-2.0-only WITH Linux-syscall-note */

#ifndef _UAPI_ASM_WASM_SIGCONTEXT_H
#define _UAPI_ASM_WASM_SIGCONTEXT_H

#include <uapi/asm/ptrace.h>

/* State saved before a signal is handled, given to signal handlers. */
struct sigcontext {
	struct user_regs_struct regs;
};

#endif /* _UAPI_ASM_WASM_SIGCONTEXT_H */
