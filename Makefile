# Makefile for bananian (installer)



OUTPUTS = initrd.img boot.img debroot debroot.tar
# Pass USE_QEMU=1 to the make command to bootstrap on the build machine
USE_QEMU = 0
ifeq ($(USE_QEMU),1)
ONDEV_BOOTSTRAP_CMD =
USE_QEMU_INSTALL = qemu-install
else
ONDEV_BOOTSTRAP_CMD = adb shell /data/bootstrap-debian.sh
USE_QEMU_INSTALL =
endif
DEFAULT_PACKAGES = openssh-server,vim,wpasupplicant,man-db,busybox,sudo,$(EXTRA_PACKAGES)

all: check $(OUTPUTS)

VERSION=0.2
export VERSION
DEBS = bananui-base_$(VERSION)_armhf.deb device-startup_$(VERSION)_all.deb

LDLAGS = -B "$$(pwd)/debroot/usr/lib/arm-linux-gnueabihf/" \
	-B "$$(pwd)/libbananui/" \
	-Wl,-rpath-link="$$(pwd)/debroot/usr/lib/arm-linux-gnueabihf/"
CFLAGS = -isystem "$$(pwd)/debroot/usr/include/arm-linux-gnueabihf/" \
	-isystem "$$(pwd)/debroot/usr/include/" \
	-isystem "$$(pwd)/sysincludes/"

export CFLAGS

getversion:
	@echo "$(VERSION)"

check::
	@scripts/check packages bananui-base device-startup libbananui \
		libbananui-dev
	@scripts/check root
	@scripts/check deps

bananui-base_$(VERSION)_armhf.deb: bananui-base libbananui-debs
	echo "$(VERSION)" > bananui-base/.version
	if [ ! -f bananui-base/.prebuilt ]; then \
		(cd bananui-base; pdebuild --configfile ../pbuilderrc \
		-- --host-arch armhf \
		--override-config --othermirror \
		"deb [trusted=yes] file:///$$(pwd)/../libbananui-debs ./") \
	fi

device-startup_$(VERSION)_all.deb: device-startup
	if [ ! -f device-startup/.prebuilt ]; then \
		(cd device-startup; pdebuild --configfile ../pbuilderrc \
		-- --host-arch armhf); \
	fi

libbananui-debs: libbananui0_$(VERSION)_armhf.deb
	mkdir libbananui-debs; cp ../libbananui0*_$(VERSION)_armhf.deb \
		libbananui-debs/; \
	dpkg-scanpackages libbananui-debs /dev/null > \
		libbananui-debs/Packages;

libbananui0_$(VERSION)_armhf.deb: libbananui
	if [ ! -f libbananui/.prebuilt ]; then \
		(cd libbananui; pdebuild --configfile ../pbuilderrc \
			-- --host-arch armhf) \
	fi

initrd.img: ramdisk
	rm -f $@
	scripts/pack-initrd $@ $<

boot.img: initrd.img zImage bootimg.cfg
	abootimg --create $@ -f bootimg.cfg -k zImage -r $<

debroot:
	rm -rf debroot
	debootstrap --include=$(DEFAULT_PACKAGES) --arch armhf --foreign \
		buster debroot/ $(MIRROR) || rm -rf debroot

copy-files: $(DEBS) modules
	[ ! -f debroot/etc/wpa_supplicant.conf ] && \
		echo 'network={' >> debroot/etc/wpa_supplicant.conf && \
		echo '    ssid="SSID"' >> debroot/etc/wpa_supplicant.conf && \
		echo '    psk="PSK"' >> debroot/etc/wpa_supplicant.conf && \
		echo '}' >> debroot/etc/wpa_supplicant.conf
	editor debroot/etc/wpa_supplicant.conf
	mkdir -p debroot/lib/modules/
	rm -rf debroot/lib/modules/3.10.49-bananian+
	cp -rf modules debroot/lib/modules/3.10.49-bananian+
	cp -f $(DEBS) libbananui_$(VERSION)_armhf.deb debroot/var/cache

ifeq ($(USE_QEMU),1)
qemu-install: debroot copy-files
	scripts/qemubootstrap
endif

debroot.tar: debroot copy-files $(USE_QEMU_INSTALL)
	rm -f $@
	(cd debroot; tar cvf ../$@ --exclude=.gitignore *)
	@echo "Now you can execute the commands from README.md."

install-to-device: all
	adb wait-for-device
	adb push debroot.tar /data
	adb push boot.img /data
	adb push on-device-scripts/*.sh /data
	adb shell /data/install-bootimage.sh
	adb shell /data/unpack-debian.sh
	$(ONDEV_BOOTSTRAP_CMD)
	adb shell rm /data/install-bootimage.sh /data/unpack-debian.sh \
		/data/bootstrap-debian.sh

clean:
	rm -rf *.deb $(OUTPUTS) libbananui-debs
