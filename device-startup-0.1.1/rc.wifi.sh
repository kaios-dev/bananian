#!/bin/sh
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
#
# GNU Mailutils is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
#

wifi_error () {
	echo "There was an error loading wifi."
	echo "Kernel log output:"
	dmesg
}

echo 1 > /dev/wcnss_wlan
modprobe wlan
if [ $? != 0 ]; then wifi_error; exit 1; fi
busybox ifconfig wlan0 up
busybox ifconfig wlan0
wpa_supplicant -B -Dwext -iwlan0 -c/etc/wpa_supplicant.conf \
	-P/run/wpa_supplicant.pid
dhclient wlan0
