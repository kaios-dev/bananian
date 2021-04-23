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
DEFAULT_PACKAGES = hicolor-icon-theme,adwaita-icon-theme,libgraphicsmagick-q16-3,openssh-server,vim,wpasupplicant,man-db,busybox,sudo,$(EXTRA_PACKAGES)
MIRROR = http://deb.debian.org/debian

.PHONY: all
all: .config check packages $(OUTPUTS)

VERSION=0.2.1
export VERSION
RELEASE=0
export RELEASE

.config:
	@scripts/configure

.PHONY: config
config:
	@scripts/configure

.PHONY: getversion
getversion:
	@echo "$(VERSION)"

.PHONY: check
check:
	@scripts/check root
	@scripts/check deps

libbananui-debs:
	(mkdir -p libbananui-debs; cd libbananui-debs; \
	cp ../libbananui0*_$(VERSION)_armhf.deb .; \
	dpkg-scanpackages . /dev/null > Packages)

initrd.img: ramdisk
	rm -f $@
	scripts/pack-initrd $@ $<

boot.img: initrd.img zImage bootimg.cfg
	abootimg --create $@ -f bootimg.cfg -k zImage -r $<

debroot:
	rm -rf debroot
	debootstrap --include=$(DEFAULT_PACKAGES) --arch armhf --foreign \
		--merged-usr buster debroot/ $(MIRROR) || \
		(rm -rf debroot && false)

.PHONY: download
download: .config
	@scripts/download-packages

.PHONY: packages
packages: download
	@scripts/build-packages

.PHONY: copy-files
copy-files: packages modules
	if [ ! -f debroot/etc/wpa_supplicant.conf ]; then \
		echo 'network={' >> debroot/etc/wpa_supplicant.conf && \
		echo '    ssid="SSID"' >> debroot/etc/wpa_supplicant.conf && \
		echo '    psk="PSK"' >> debroot/etc/wpa_supplicant.conf && \
		echo '}' >> debroot/etc/wpa_supplicant.conf; \
	fi
	editor debroot/etc/wpa_supplicant.conf
	mkdir -p debroot/lib/modules/
	rm -rf debroot/lib/modules/3.10.49-bananian+
	cp -rf modules debroot/lib/modules/3.10.49-bananian+
	cp -f $$(cat .packages) debroot/var/cache

.PHONY: package
ifeq ($(PACKAGE_PATH),)
package:
	@echo "Please set the PACKAGE_PATH variable!"; exit 1
else
ifeq ($(NO_LIBBANANUI_DEPEND),1)
package:
	@echo "Building package..."
	cd '$(PACKAGE_PATH)' && \
	pdebuild --configfile '$(CURDIR)/pbuilderrc' \
		--buildresult '$(CURDIR)' -- --host-arch armhf
else
package: libbananui-debs
	@echo "Building package..."
	TMPDEBS=$$(mktemp -d /tmp/libbananui-debs.XXXXXXXX) && \
	cp -r libbananui-debs/* "$$TMPDEBS" && \
	cd '$(PACKAGE_PATH)' && \
	pdebuild --configfile '$(CURDIR)/pbuilderrc' \
		--buildresult '$(CURDIR)' -- --host-arch armhf \
		--bindmounts "$$TMPDEBS" \
		--override-config --othermirror \
		"deb [trusted=yes] file://$$TMPDEBS ./"; \
	rm -rf "$$TMPDEBS"
endif
endif

ifeq ($(USE_QEMU),1)
.PHONY: qemu-install
qemu-install: debroot copy-files sysconfig
	@scripts/qemubootstrap
endif

.PHONY: sysconfig
sysconfig: debroot
	@scripts/sysconfig

debroot.tar: debroot copy-files sysconfig $(USE_QEMU_INSTALL)
	rm -f $@
	(cd debroot; tar cvf ../$@ --exclude=.gitignore *)
	@echo "Now you can execute the commands from README.md."

.PHONY: install-to-device
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

.PHONY: clean
clean:
	rm -rf *.deb $(OUTPUTS) libbananui-debs
