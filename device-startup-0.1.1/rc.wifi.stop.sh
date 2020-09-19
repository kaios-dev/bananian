#!/bin/sh
if [ -f /run/wpa_supplicant.pid ]; then
	kill `cat /run/wpa_supplicant.pid`
fi
rmmod wlan
