# Makefile for bananian (installer)



OUTPUTS = initrd.img boot.img debroot debroot.tar
DEFAULT_PACKAGES = openssh-server,vim,wpasupplicant,man-db,busybox,sudo,$(EXTRA_PACKAGES)

all: check $(OUTPUTS)

VERSION = $(shell git describe --abbrev=0)
export VERSION
DEBS = bananui-base_$(VERSION)_armhf.deb device-startup_$(VERSION)_all.deb

getversion:
	@echo "$(VERSION)"

check::
	@./check packages bananui-base device-startup
	@./check root
	@./check deps
	@./check gcc

bananui-base_$(VERSION)_armhf.deb: bananui-base
	echo "$(VERSION)" > bananui-base/.version
	(cd bananui-base; debuild --no-lintian -us -uc -aarmhf)

device-startup_$(VERSION)_all.deb: device-startup
	(cd device-startup; debuild --no-lintian -us -uc -aarmhf)

package-%: %
	(cd $^; debuild --no-lintian -us -uc -aarmhf)

initrd.img: ramdisk
	rm -f $@
	./pack-initrd $@ $<

boot.img: initrd.img zImage bootimg.cfg
	abootimg --create $@ -f bootimg.cfg -k zImage -r $<

debroot:
	rm -rf debroot
	debootstrap --include=$(DEFAULT_PACKAGES) --arch armhf --foreign \
		buster debroot/ $(MIRROR) || rm -rf debroot
	mkdir -p debroot/lib/modules/
	cp -rf modules debroot/lib/modules/3.10.49-bananian+

debroot.tar: debroot $(DEBS)
	rm -f $@
	cp -f $(DEBS) debroot/var/cache
	(cd debroot; tar cvf ../$@ --exclude=.gitignore *)
	@echo "Now you can execute the commands from README.md."

clean:
	rm -rf *.deb $(OUTPUTS)
