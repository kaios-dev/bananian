# Makefile for bananian (installer)



OUTPUTS = initrd.img boot.img debroot debroot.tar
# Pass USE_QEMU=1 to the make command to bootstrap on the build machine
USE_QEMU = 0
ifeq ($(USE_QEMU),1)
ONDEV_BOOTSTRAP_CMD =
USE_QEMU_INSTALL = qemu-install
else
ONDEV_BOOTSTRAP_CMD = @adb shell /data/bootstrap-debian.sh
USE_QEMU_INSTALL =
endif
DEFAULT_PACKAGES = hicolor-icon-theme,adwaita-icon-theme,libgraphicsmagick-q16-3,openssh-server,vim,wpasupplicant,man-db,busybox,sudo,$(EXTRA_PACKAGES)
MIRROR = http://deb.debian.org/debian

ifeq ($(wildcard .config),)
.config: menuconfig
endif

.PHONY: all
all: .config check packages $(OUTPUTS)

VERSION=0.2.1
export VERSION
RELEASE=0
export RELEASE

CONFIG_OBJ = $(CURDIR)/scripts/config
CONFIG_IN = $(CURDIR)/Config.in

.PHONY: config menuconfig xconfig gconfig nconfig
$(CONFIG_OBJ)/%onf:
	mkdir -p $(@D)/lxdialog
	$(MAKE) -C scripts/kconfig -f Makefile.br obj=$(@D) $(@F) \
		HOSTCC=$(CC) HOSTCXX=$(CXX)

generate-package-configs:
	@scripts/generate-package-configs

menuconfig: $(CONFIG_OBJ)/mconf generate-package-configs
	@$< $(CONFIG_IN)
config: $(CONFIG_OBJ)/conf generate-package-configs
	@$< $(CONFIG_IN)
xconfig: $(CONFIG_OBJ)/qconf generate-package-configs
	@$< $(CONFIG_IN)
gconfig: $(CONFIG_OBJ)/gconf generate-package-configs
	@$< $(CONFIG_IN)
nconfig: $(CONFIG_OBJ)/nconf generate-package-configs
	@$< $(CONFIG_IN)

help:
	@echo 'Cleaning targets:'
	@echo '  clean              - Remove packages, root and other generated'
	@echo '                       files, excluding configuration'
	@echo ''
	@echo 'Configuration targets:'
	@echo '  config             - Update current config utilising a line-oriented program'
	@echo '  nconfig            - Update current config utilising a ncurses menu based'
	@echo '                       program'
	@echo '  menuconfig         - Update current config utilising a menu based program'
	@echo '  xconfig            - Update current config utilising a Qt based front-end'
	@echo '  gconfig            - Update current config utilising a GTK+ based front-end'

	@echo ''
	@echo 'Installation targets:'
	@echo '  install-to-device  - Install Bananian via adb'

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
	@scripts/copy-packages debroot/var/cache

.PHONY: package
ifeq ($(PACKAGE_PATH),)
package:
	@echo "Please set the PACKAGE_PATH variable!"; exit 1
else
ifeq ($(NO_LIBBANANUI_DEPEND),1)
package:
	@echo "Building package in $(PACKAGE_PATH)..."
	@cd '$(PACKAGE_PATH)' && echo 'Do not need to copy libbananui' && \
	pdebuild --configfile '$(CURDIR)/pbuilderrc' \
		--buildresult '$(CURDIR)' -- --host-arch armhf
else
package: libbananui-debs
	@echo "Building package in $(PACKAGE_PATH)..."
	@TMPDEBS=$$(mktemp -d /tmp/libbananui-debs.XXXXXXXX) && \
	echo 'Copying libbananui...' && cp -r libbananui-debs/* "$$TMPDEBS" && \
	cd '$(PACKAGE_PATH)' && \
	pdebuild --configfile '$(CURDIR)/pbuilderrc' \
		--buildresult '$(CURDIR)' -- --host-arch armhf \
		--bindmounts "$$TMPDEBS" \
		--override-config --othermirror \
		"deb [trusted=yes] file://$$TMPDEBS ./"; pdebuildresult=$$?; \
	rm -rf "$$TMPDEBS"; exit $$pdebuildresult
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
	@(echo '==>> Creating debroot.tar...' && cd debroot && \
		tar cf ../$@ --exclude=.gitignore *)
	@echo "Now you can execute the commands from README.md."

.PHONY: install-to-device
install-to-device: all
	adb wait-for-device
	@adb push debroot.tar /data
	@adb push boot.img /data
	@adb push on-device-scripts/*.sh /data
	@adb shell /data/install-bootimage.sh
	@adb shell /data/unpack-debian.sh
	$(ONDEV_BOOTSTRAP_CMD)
	@adb shell rm /data/install-bootimage.sh /data/unpack-debian.sh \
		/data/bootstrap-debian.sh

.PHONY: clean
clean:
	rm -rf *.deb $(OUTPUTS) libbananui-debs
