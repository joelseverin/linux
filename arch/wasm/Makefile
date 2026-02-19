# SPDX-License-Identifier: GPL-2.0-only

KBUILD_DEFCONFIG := wasm32_nommu_defconfig

KCFLAGS += -nostdlib -fno-builtin

ifeq ($(CONFIG_64BIT),y)
	KCFLAGS += -march=wasm64
	KAFLAGS += -march=wasm64
	KBUILD_LDFLAGS += -mwasm64
else
	KCFLAGS += -march=wasm32
	KAFLAGS += -march=wasm32
	KBUILD_LDFLAGS += -mwasm32
endif

core-y += arch/wasm/kernel/
core-y += arch/wasm/mm/
libs-y += arch/wasm/lib/

define archhelp
	echo  '* vmlinux        - Standard Wasm kernel'
endef
