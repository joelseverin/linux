/* SPDX-License-Identifier: GPL-2.0-only */

#include <linux/export.h>
#include <linux/types.h>

/* Wasm has no i128 but LLVM supports it as 2x i64, normally via compiler-rt. */
__visible u128 __multi3(u128 a, u128 b);
__visible u128 __multi3(u128 a, u128 b)
{
    u128 aa = a;
    u128 bb = b;
    u128 acc = 0;

    /* TODO: This could be made much more efficiently with u64:s... */
    while (aa) {
        if (aa & 1) {
            acc += bb;
        }
        aa >>= 1;
        bb <<= 1;
    }

    return acc;
}
EXPORT_SYMBOL(__multi3);
