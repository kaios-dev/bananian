#!/system/bin/sh
# Copyright (C) 2020-2021 Affe Null <affenull2345@gmail.com>
# 
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
#

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
