#!/system/bin/sh

set -e

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

echo "[on device] ==>> Mounting $DEBIANDIR"

cd /data
if [ -d "$DEBIANDIR" ]; then
	echo "[on device] ===>> ERROR! A debian directory has been found!" \
		"Please unmount/delete it."
	exit 1
fi
mkdir "$DEBIANDIR"
busybox mount "$DEVICE" "$DEBIANDIR" || true
cd "$DEBIANDIR"
allfiles=*
if [ "x$allfiles" != "x*" ]; then
	echo "[on device] ===>> ERROR! Debian partition not empty! Please" \
		"delete all files manually."
	exit 1
fi
echo '[ on device ] ==>> Unpacking debroot.tar'
tar xf ../debroot.tar
