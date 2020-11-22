#!/system/bin/sh

dd if=/data/boot.img of=/dev/block/bootdevice/by-name/recovery bs=2048
