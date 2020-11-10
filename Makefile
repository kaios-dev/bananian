# Makefile for bananian (installer)



VERSION = 0.1.1
OUTPUTS = initrd.img boot.img debroot debroot.tar
DEFAULT_PACKAGES = openssh-server,vim,wpasupplicant,man-db,busybox,sudo,$(EXTRA_PACKAGES)
DEBS = bananui-base_$(VERSION)_armhf.deb device-startup_$(VERSION)_all.deb

all: check $(OUTPUTS)

check::
	@./check packages
	@./check deps
	@./check gcc

bananui-base_$(VERSION)_armhf.deb: bananui-base
	echo "$(VERSION)" > bananui-base/.version
	(cd bananui-base; debuild --no-lintian -us -uc -aarmhf)

device-startup_$(VERSION)_all.deb: device-startup
	(cd bananui-base; debuild --no-lintian -us -uc -aarmhf)

initrd.img: ramdisk
	rm -f $@
	./pack-initrd $@ $<

boot.img: initrd.img zImage bootimg.cfg
	abootimg --create $@ -f bootimg.cfg -k zImage -r $<

debroot:
	rm -rf debroot
	debootstrap --include=$(DEFAULT_PACKAGES) --arch armhf --foreign \
		buster debroot/ $(MIRROR)
	mkdir -p debroot/lib/modules/
	cp -rf modules debroot/lib/modules/3.10.49-bananian

debroot.tar: debroot $(DEBS)
	rm -f $@
	cp -f $(DEBS) debroot/var/cache
	(cd debroot; tar cvf ../$@ --exclude=.gitignore *)
	@echo "Now you can execute the commands from README.md."

clean:
	rm -rf *.deb $(OUTPUTS)
