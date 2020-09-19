# Bananian - Debian for the Nokia 8110 4G and a UI
### Compiling
Just run make. If it complains about missing compilers, make sure your ARM
cross-compiler is named arm-linux-gnueabi-gcc or arm-linux-gnueabi-gcc-_version_.
### Installing
This step requires a rooted phone. See
[BananaHackers](https://sites.google.com/view/bananahackers/root) for more info.
Create two partitions on your SD card. Format the first one as FAT and the
second as EXT4. Push boot.img to the phone and flash it to boot or recovery:

    (on your phone)
    # dd if=path/to/boot.img of=/dev/block/bootdevice/by-name/<boot or recovery>

Then insert the SD card into the phone, push the debroot.tar file and run the
following commands on your phone:

    # mkdir /data/debian
    # busybox mount /dev/block/mmcblk1p2 /data/debian
    # cd /data/debian
    # tar xvf /path/to/debroot.tar
    (lots of output)
    # busybox umount /data/debian

### Wireless networking
To enable WiFi networking, create a file named debroot/etc/wpa\_supplicant.conf
after compiling and add the following contents:

    network={
        ssid="[your network name]"
        psk="[your network password]"
    }

### Shell access
You need to enable networking first.
To get shell access to your phone, find out its IP address via your router. Then:

    $ ssh root@X.X.X.X # Where X.X.X.X is your the phone's IP address

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
