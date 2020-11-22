# Makefile for bananian (installer)



OUTPUTS = initrd.img boot.img debroot debroot.tar
# Pass USE_QEMU=1 to the make command to bootstrap on the build machine
USE_QEMU = 0
ifeq ($(USE_QEMU),1)
QEMU_CMD = scripts/qemubootstrap
else
QEMU_CMD =
endif
DEFAULT_PACKAGES = openssh-server,vim,wpasupplicant,man-db,busybox,sudo,$(EXTRA_PACKAGES)

all: check $(OUTPUTS)

VERSION=$(shell git describe --tags --abbrev=0)
export VERSION
DEBS = bananui-base_$(VERSION)_armhf.deb device-startup_$(VERSION)_all.deb

getversion:
	@echo "$(VERSION)"

check::
	@scripts/check packages bananui-base device-startup
	@scripts/check root
	@scripts/check deps
	@scripts/check gcc

bananui-base_$(VERSION)_armhf.deb: bananui-base
	echo "$(VERSION)" > bananui-base/.version
	(cd bananui-base; debuild --no-lintian -us -uc -aarmhf)

device-startup_$(VERSION)_all.deb: device-startup
	(cd device-startup; debuild --no-lintian -us -uc -aarmhf)

package-%: %
	(cd $^; debuild --no-lintian -us -uc -aarmhf)

initrd.img: ramdisk
	rm -f $@
	scripts/pack-initrd $@ $<

boot.img: initrd.img zImage bootimg.cfg
	abootimg --create $@ -f bootimg.cfg -k zImage -r $<

debroot:
	rm -rf debroot
	debootstrap --include=$(DEFAULT_PACKAGES) --arch armhf --foreign \
		buster debroot/ $(MIRROR) || rm -rf debroot
	mkdir -p debroot/lib/modules/
	cp -rf modules debroot/lib/modules/3.10.49-bananian+
	cp -f $(DEBS) debroot/var/cache
	$(QEMU_CMD)

ifeq ($(USE_QEMU),1)
qemu-retry:
	mkdir -p debroot/lib/modules/
	cp -rf modules debroot/lib/modules/3.10.49-bananian+
	cp -f $(DEBS) debroot/var/cache
	$(QEMU_CMD)
endif

debroot.tar: debroot $(DEBS)
	rm -f $@
	(cd debroot; tar cvf ../$@ --exclude=.gitignore *)
	@echo "Now you can execute the commands from README.md."

clean:
	rm -rf *.deb $(OUTPUTS)
