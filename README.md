# Bananian - Debian for the Nokia 8110 4G and a UI
### Building
NOTE: This has been tested only on Debian GNU/Linux. There will
probably be many errors on non-Debian-based GNU/Linux distros.

To build the subpackages, clone the package repositories:

    $ git clone https://gitlab.com/affenull2345/bananui-base
    $ git clone https://gitlab.com/affenull2345/device-startup

If you don't do this, the build system will try downloading prebuilt versions
of the packages. It may fail if the version is too old. Older versions can be
found at:
 * (prebuilt) <https://gitlab.com/affenull2345/bananian/-/packages>
 * (source) <https://gitlab.com/affenull2345/bananui-base/-/releases> and
   <https://gitlab.com/affenull2345/device-startup/-/releases>

Then run make as root (Please make a dry-run first to prove that nothing
malicious happens). If it complains about missing compilers, make sure that
the package crossbuild-essential-armhf is installed.

#### QEMU mode
QEMU mode runs debootstrap --second-stage and various other setup commands in
the qemu-user emulator. The package qemu-user-static is required for this mode.
To enable it, append USE\_QEMU=1 to the make or make install-to-device command:

    # make USE_QEMU=1 && make install-to-device USE_QEMU=1

#### Wireless networking
During the build process, you will be prompted to edit a file named
/etc/wpa\_supplicant.conf. In case you want to install manually, here is its
format:

    network={
        ssid="[your network name]"
        psk="[your network password]"
    }

### Installing
This step requires a rooted phone. See
[BananaHackers](https://sites.google.com/view/bananahackers/root) for more info.
Create two partitions on your SD card. Format the first one as FAT and the
second as EXT4.
Now you can run make install-to-device and it will install it automatically.
Alternatively, you can install Bananian manually:

Push boot.img to the phone and flash it to boot or recovery:

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
    (The following commands before 'exit' are only needed if QEMU mode was
    disabled.)
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
    adduser user sudo
    (These commands are always needed:)
    exit
    dd if=/path/to/boot.img" \
		"of=/dev/block/bootdevice/by-name/<recovery or boot> bs=2048
    <reboot or reboot recovery>

### Passwords

If you used the on-device bootstrap method, the password will be set to
the default value 'bananian'.
If you use the QEMU bootstrap method, the password will be set at build time.

### Shell access
To get shell access to your phone, find out its IP address via your router. Then:

    $ ssh user@X.X.X.X # Where X.X.X.X is your the phone's IP address

To get root access, use sudo.

### Window list
To show a list of open windows, press the power button with the slide open.
### Bugs
There some bugs and many things aren't implemented yet.
Here is one bug:
 - Color Grid app does not always work (it has been removed from the home screen
but can be launched via ssh as colorgrid)

Please report other bugs as issues.
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
