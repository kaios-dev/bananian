#!/bin/sh

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
