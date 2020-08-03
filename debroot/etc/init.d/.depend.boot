TARGETS = mountkernfs.sh hostname.sh udev mountdevsubfs.sh hwclock.sh checkroot.sh mountnfs.sh mountnfs-bootclean.sh mountall.sh mountall-bootclean.sh networking urandom brightness kmod procps checkroot-bootclean.sh bootmisc.sh checkfs.sh
INTERACTIVE = udev checkroot.sh checkfs.sh
udev: mountkernfs.sh
mountdevsubfs.sh: udev
hwclock.sh: mountdevsubfs.sh
checkroot.sh: mountdevsubfs.sh hostname.sh hwclock.sh
mountnfs.sh: mountall.sh mountall-bootclean.sh networking
mountnfs-bootclean.sh: mountall.sh mountall-bootclean.sh mountnfs.sh
mountall.sh: checkfs.sh checkroot-bootclean.sh
mountall-bootclean.sh: mountall.sh
networking: mountkernfs.sh mountall.sh mountall-bootclean.sh urandom procps
urandom: mountall.sh mountall-bootclean.sh hwclock.sh
brightness: mountall.sh mountall-bootclean.sh
kmod: checkroot.sh
procps: udev mountall.sh mountall-bootclean.sh
checkroot-bootclean.sh: checkroot.sh
bootmisc.sh: udev checkroot-bootclean.sh mountall-bootclean.sh mountnfs-bootclean.sh mountnfs.sh mountall.sh
checkfs.sh: checkroot.sh
