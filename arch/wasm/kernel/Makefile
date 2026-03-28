# SPDX-License-Identifier: GPL-2.0-only

extra-y += vmlinux.lds

obj-y += cpu.o
obj-y += cpuflags.o
obj-y += entry.o
obj-y += head.o
obj-y += irqflags.o
obj-y += irq.o
obj-y += process.o
obj-y += ptrace.o
obj-y += reboot.o
obj-y += setup.o
obj-y += signal.o
obj-y += smp.o
obj-y += sys_wasm.o
obj-y += syscall_table.o
obj-y += time.o
obj-y += traps.o

ifndef CONFIG_64BIT
obj-y += sys_wasm32.o
endif
