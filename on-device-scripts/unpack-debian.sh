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

cd /data
if [ -d "$DEBIANDIR" ]; then
	echo "WARNING! A debian directory has been found! Please " \
		"unmount/delete it."
	exit 1
fi
mkdir "$DEBIANDIR"
busybox mount "$DEVICE" "$DEBIANDIR" || true
cd "$DEBIANDIR"
allfiles=*
if [ "x$allfiles" = "x*" ]; then
	echo "Debian partition not empty! Please delete all files manually."
	exit 1
fi
tar xvf ../debroot.tar
