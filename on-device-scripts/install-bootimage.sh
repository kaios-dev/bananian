#!/system/bin/sh

echo "[on device] ==>> Flashing recovery..."
dd if=/data/boot.img of=/dev/block/bootdevice/by-name/recovery bs=2048
