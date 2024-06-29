/* SPDX-License-Identifier: GPL-2.0-only */

#include "../../../drivers/tty/hvc/hvc_console.h"

extern ssize_t wasm_driver_hvc_put(const u8 *buf, size_t count);
extern ssize_t wasm_driver_hvc_get(u8 *buf, size_t count);

static ssize_t hvc_wasm_put_chars(uint32_t vtermno, const u8 *buf, size_t count)
{
	return wasm_driver_hvc_put(buf, count);
}

static ssize_t hvc_wasm_get_chars(uint32_t vtermno, u8 *buf, size_t count)
{
	return wasm_driver_hvc_get(buf, count);
}

static const struct hv_ops hvc_wasm_ops = {
	.get_chars = hvc_wasm_get_chars,
	.put_chars = hvc_wasm_put_chars,
};

static int __init hvc_wasm_init(void)
{
	return PTR_ERR_OR_ZERO(hvc_alloc(0, 0, &hvc_wasm_ops, PAGE_SIZE));
}
device_initcall(hvc_wasm_init);

static int __init hvc_wasm_console_init(void)
{
	hvc_instantiate(0, 0, &hvc_wasm_ops);

	return 0;
}
console_initcall(hvc_wasm_console_init);
