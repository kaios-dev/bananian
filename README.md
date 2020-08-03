# Bananian - Debian for the Nokia 8110 4G and a UI
### Compiling
Just run make. If it complains about missing compilers, make sure your ARM
cross-compiler is named arm-linux-gnueabi-gcc or arm-linux-gnueabi-gcc-_version_.
### Installing
Create two partitions on your SD card. Format the first one as FAT and the
second as EXT4. Flash boot.img to boot or recovery.
### Wireless networking
To enable WiFi networking, create a file named debroot/etc/wpa\_supplicant.conf
before compiling and add the following contents:
    network={
        ssid="[your network name]"
        psk="[your network password]"
    }
### Bugs
There some bugs and many things aren't implemented yet.
Here is one bug:
 - The display is very glitchy because some parts don't get refreshed.
Please report other bugs as an issue.
