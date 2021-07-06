The kernel binaries: zImage are licensed under GNU GPL version 2.0
WITH Linux-syscall-note. See KERNEL.md for information on how/where to obtain
the source code and build it.

The device tree blob (`dtb`) was also built from the kernel source by setting
`TARGET_PRODUCT=argon`.

The binary in `ramdisk/sbin/busybox` is the official BusyBox armv7 binary.
BusyBox is licensed under the GNU GPL version 2.0. See <https://busybox.net/>
for more information. Source code can be downloaded from 
<https://busybox.net/downloads/busybox-1.31.0.tar.bz2>.

The modules in `modules/` are part of the kernel and their
licenses are included in the kernel source tree.

The `modules/kernel/drivers/net/wireless/prima/wlan.ko` module is distributed
out-of-tree under the terms of the ISC license. KERNEL.md also contains a link
to a copy of its source code.

The GNU GPL version 3 only applies to the files with a notice that
they are licensed under the GNU GPL version 3, such as the Makefile and some of
the installer scripts.
