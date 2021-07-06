# Install dependencies
Install build tools (skip if already installed):

    $ sudo apt install build-essential

Note: this example assumes you are using a Debian-based Linux distro. If you
are using some other system, it probably has a different package manager. The
package names might also differ, so do a package search first.

Install GCC 4.9.4 dependencies:

    $ sudo apt install libgmp-dev libmpc-dev libmpfr-dev crossbuild-essential-armhf

Download GCC 4.9.4 <ftp://ftp.gnu.org/gnu/gcc/gcc-4.9.4/gcc-4.9.4.tar.gz>
into your current directory

Compile GCC 4.9.4:

    $ tar xf gcc-4.9.4.tar.gz
    $ cd gcc-4.9.4
    $ mkdir bld
    $ cd bld
    $ CFLAGS_FOR_TARGET='-g -O2 -mfloat-abi=hard -D__ARM_PCS_VFP' \
      $(pwd)/../configure --prefix=/usr \
      --target=arm-linux-unknown-gnueabi \
      --enable-languages=c,c++
    $ make all-gcc
    $ sudo make install-gcc
    $ cd ..

# Compiling the official stock kernel (new method)

Download the kernel from https://www.nokia.com/phones/en\_int/opensource.

Add Prima WLAN kernel module:

    $ cd kernel/drivers/net/wireless
    $ git clone https://gitlab.com/affenull2345/prima-wlan prima
    $ echo 'obj-$(CONFIG_PRIMA_WLAN) += prima/' >> Makefile
    $ echo 'source "drivers/net/wireless/prima/Kconfig"' >> Kconfig
    $ cd ../../../..

Compile kernel:

    $ cd kernel
    $ cp kernel-config .config
    $ make menuconfig ARCH=arm
      < add/remove some configuration options >
    $ make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf-
    $ cat dtb arch/arm/boot/zImage > zImage

The zImage file is the kernel.
Since this is the official stock kernel and the configuration is based on
/proc/config.gz on the phone, this kernel should work with KaiOS.
Note that you have to replace the modules in /system/lib/modules if you want to
use it with KaiOS.

# Compiling the CodeAurora kernel (old method)

Download kernel:

    $ git clone -b LF.BR.1.2.8 https://source.codeaurora.org/quic/la/kernel/msm-3.10 kernel

Download <https://gitlab.com/affenull2345/bananian/-/raw/master/kernel-config>
into the kernel.

Add Prima WLAN kernel module:

    $ cd kernel/drivers/net/wireless
    $ git clone https://gitlab.com/affenull2345/prima-wlan prima
    $ echo 'obj-$(CONFIG_PRIMA) += prima/' >> Makefile
    $ echo 'source "drivers/net/wireless/prima/Kconfig"' >> Kconfig
    $ cd ../../../..

Compile kernel:

    $ cd kernel
    $ cp kernel-config .config
    $ make menuconfig ARCH=arm
      < add/remove some configuration options >
    $ make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf-
    $ cat dtb arch/arm/boot/zImage > zImage

The zImage file is the kernel, you can now put it into a bootimage and flash it.
WARNING: This kernel does not work with KaiOS. It crashes on boot.
