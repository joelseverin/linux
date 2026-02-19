/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef _ASM_WASM_TIME_H
#define _ASM_WASM_TIME_H

void wasm_clockevent_enable(void);
void wasm_program_timer(unsigned long delta);

#endif /* _ASM_WASM_TIME_H */
