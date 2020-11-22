#!/system/bin/sh
DEBIANDIR="debian"
DEVICE="/dev/block/mmcblk1p2"

if [ "$1" = "-d" ]; then
	shift
	DEBIAN="$1"
	shift
fi
if [ "$1" = "-p" ]; then
	shift
	DEVICE="$1"
	shift
fi

cd /data
busybox mount "$DEVICE" "$DEBIANDIR"
busybox mount -t devtmpfs devtmpfs "$DEBIANDIR/dev" || \
	mount -o bind /dev "$DEBIANDIR/dev"
for i in dev/pts proc sys data system sdcard; do busybox mount -o bind /$i \
	"$DEBIANDIR/$i"; done

TERM=xterm SHELL=/bin/bash HOME=/root PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:$PATH chroot "$DEBIANDIR" $@
