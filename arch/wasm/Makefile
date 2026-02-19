# SPDX-License-Identifier: GPL-2.0-only

KBUILD_DEFCONFIG := wasm_defconfig

KCFLAGS += -nostdlib -fno-builtin

ifdef CONFIG_64BIT
	KCFLAGS += -march=wasm64
else
	KCFLAGS += -march=wasm32
endif

# The kernel's export system does not work in Wasm.
KCFLAGS += -D__DISABLE_EXPORTS

core-y += arch/wasm/kernel/
core-y += arch/wasm/mm/
libs-y += arch/wasm/lib/

PHONY += bzImage

all: bzImage

bzImage: vmlinux

define archhelp
	echo  '* bzImage     - Compressed kernel image (arch/wasm/boot/bzImage)'
endef
