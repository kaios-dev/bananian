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

if [ "$#" -gt 0 ]; then
	cmd="$@"
else
	cmd="/bin/bash"
fi

TERM=xterm SHELL=/bin/bash HOME=/root PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:$PATH chroot "$DEBIANDIR" /bin/sh -c "$cmd"
