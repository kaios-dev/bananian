TARGETS = mountkernfs.sh hostname.sh udev mountdevsubfs.sh mountnfs.sh mountnfs-bootclean.sh mountall.sh mountall-bootclean.sh hwclock.sh networking urandom brightness checkroot.sh checkfs.sh checkroot-bootclean.sh kmod procps bootmisc.sh
INTERACTIVE = udev checkroot.sh checkfs.sh
udev: mountkernfs.sh
mountdevsubfs.sh: udev
mountnfs.sh: mountall.sh mountall-bootclean.sh networking
mountnfs-bootclean.sh: mountall.sh mountall-bootclean.sh mountnfs.sh
mountall.sh: checkfs.sh checkroot-bootclean.sh
mountall-bootclean.sh: mountall.sh
hwclock.sh: mountdevsubfs.sh
networking: mountkernfs.sh mountall.sh mountall-bootclean.sh urandom procps
urandom: mountall.sh mountall-bootclean.sh hwclock.sh
brightness: mountall.sh mountall-bootclean.sh
checkroot.sh: mountdevsubfs.sh hostname.sh hwclock.sh
checkfs.sh: checkroot.sh
checkroot-bootclean.sh: checkroot.sh
kmod: checkroot.sh
procps: udev mountall.sh mountall-bootclean.sh
bootmisc.sh: udev checkroot-bootclean.sh mountnfs.sh mountnfs-bootclean.sh mountall.sh mountall-bootclean.sh
