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
before compiling and add the following contents:

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
 - The display is very glitchy because some parts don't get refreshed.
 - Color Grid app does not always work

Please report other bugs as an issue.
### Contribute
If you would like to contribute, you can always submit a merge request.
If you want to write an app, check out some of the existing apps and also the
[Protocol Documentation](https://affenull2345.gitlab.io/bananian/Bananui-Protocol.html).
### Kernel
The kernel source is located at
<https://source.codeaurora.org/quic/la/kernel/msm-3.10> under branch LF.BR.1.2.8.

Commit hash: e89da83520e1225d61d03ca39dd0b5009c0b892d

Configuration (copy to .config) is in the kernel-config file.

Patches:
<https://gitlab.com/postmarketOS/pmaports/-/tree/master/device/testing/linux-nokia-beatles/>

The WLAN kernel module can be found in my repository
<https://gitlab.com/affenull2345/prima-wlan>. Add the code to
drivers/net/wireless/prima in the tree to compile it.

Device tree source: unavailable, blobs extracted from original KaiOS kernel.

Append the dtb file from this repository to get the final kernel image.
### Disclaimer
Install this at your own risk! I am not responsible for bricked phones!

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
