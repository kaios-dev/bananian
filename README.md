# Bananian - Debian for the Nokia 8110 4G and a UI
### Compiling
NOTE: This has been tested only on Debian GNU/Linux. There will
probably be many errors on non-Debian-based GNU/Linux distros.

Just run make as root (Please make a dry-run first to prove that nothing
malicious happens). If it complains about missing compilers, make sure that
the package crossbuild-essential-armhf (or similar) is installed.
### Installing
This step requires a rooted phone. See
[BananaHackers](https://sites.google.com/view/bananahackers/root) for more info.
Create two partitions on your SD card. Format the first one as FAT and the
second as EXT4. Push boot.img to the phone and flash it to boot or recovery:

    (on your phone)
    # dd if=path/to/boot.img of=/dev/block/bootdevice/by-name/<boot or recovery>

Then insert the SD card into the phone, push the debroot.tar file and run the
following commands on your phone:

    cd /data
    mkdir debroot
    busybox mount /dev/block/mmcblk1p2 debroot
    cd debroot
    busybox tar xvf /path/to/debroot.tar
     (lots of output)
    mount -o bind /dev dev
    mount -o bind /sys sys
    mount -o bind /proc proc
    export PATH=/usr/local/sbin:/usr/sbin:/usr/local/bin:/usr/bin:$PATH
    chroot . /bin/bash
    debootstrap/debootstrap --second-stage
    cd var/cache
    dpkg -i bananui-base_0.0.1-armhf.deb device-startup_0.0.1-all.deb
    adduser user
     (enter data)
    exit
    dd if=/path/to/boot.img" \
		"of=/dev/block/bootdevice/by-name/<recovery or boot> bs=2048
    reboot recovery

### Wireless networking
To enable WiFi networking, create a file named /etc/wpa\_supplicant.conf
after running debootstrap/debootstrap --second-stage:

    network={
        ssid="[your network name]"
        psk="[your network password]"
    }


### Shell access
You need to enable networking first.
To get shell access to your phone, find out its IP address via your router. Then:

    $ ssh user@X.X.X.X # Where X.X.X.X is your the phone's IP address

You should be asked for a password. The password is bananian.
### Window list
To show a list of open windows, press the power button with the slide open.
### Bugs
There some bugs and many things aren't implemented yet.
Here is one bug:
 - Color Grid app does not always work

Please report other bugs as an issue.
### Contribute
If you would like to contribute, you can always submit a merge request.
If you want to write an app, check out some of the existing apps and also the
[Protocol Documentation](https://affenull2345.gitlab.io/bananian/Bananui-Protocol.html).
### Kernel
See KERNEL.md
### Disclaimer
Install this at your own risk! I am not responsible for bricked phones!

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
