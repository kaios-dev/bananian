# Compiling a custom kernel

Download kernel:

    $ git clone -b LF.BR.1.2.8 <https://source.codeaurora.org/quic/la/kernel/msm-3.10>

Install build tools (skip if already installed):

    $ sudo apt install gcc g++ make

Note: this example assumes you are using a Debian-based Linux distro. If you
are using some other system, it probably has a different package manager. The
package names might also differ, so do a package search first.

Install GCC 4.9.4 dependencies:

    $ sudo apt install libgmp-dev libmpc-dev libmpfr-dev

Download GCC 4.9.4 <ftp://ftp.gnu.org/gnu/gcc/gcc-4.9.4/gcc-4.9.4.tar.gz>
into your current directory

Compile GCC 4.9.4:

    $ tar xf gcc-4.9.4.tar.gz
    $ cd gcc-4.9.4
    $ mkdir bld
    $ cd bld
    $ $(pwd)/../configure --prefix=/usr \
      --target=armv7l-linux-unknown-gnueabihf \
      --enable-languages=c,c++
    $ make
    $ sudo make install
    $ cd ..

Download <https://gitlab.com/affenull2345/bananian/-/raw/master/kernel-config>
into msm-3.10.
Extract device tree blob from kernel (zImage is the current kernel on the
phone):

    $ hd zImage | grep "d0 0d fe ed 00 02 14 f4  00 00 00 38 00 01 e1 f8"
    $ dd if=zImage of=msm-3.10/dtb bs=16 skip=$(0x[first hexadecimal number of
      the output from the previous command, but with one trailing zero removed])

Add Prima WLAN kernel module:

    $ cd msm-3.10/drivers/net/wireless
    $ git clone https://gitlab.com/affenull2345/prima-wlan prima
    $ echo 'obj-$(CONFIG_PRIMA) += prima/' >> Makefile
    $ echo 'source "drivers/net/wireless/prima"' >> Kconfig
    $ cd ../../../..

Compile kernel:

    $ cd msm-3.10
    $ cp kernel-config .config
    $ make menuconfig ARCH=arm
      < add/remove some configuration options >
    $ make ARCH=arm CROSS_COMPILE=armv7l-linux-gnueabihf-
    $ cat dtb arch/arm/boot/zImage > zImage

The zImage file is the kernel, you can now put it into a bootimage and flash it.
WARNING: This kernel does not work with KaiOS. It crashes on boot.
