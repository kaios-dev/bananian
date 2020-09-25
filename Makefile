VERSION = 0.1.1
OUTPUTS = initrd.img boot.img debroot debroot.tar
DEFAULT_PACKAGES = openssh-server,vim,wpasupplicant,man-db
DEBS = bananui-base_$(VERSION)-1_armhf.deb device-startup_$(VERSION)-1_all.deb

all: check-deps $(OUTPUTS)

check-deps::
	@./check-deps check
	@./check-deps checkgcc

bananui-base_$(VERSION)-1_armhf.deb: bananui-base-$(VERSION)
	echo "$(VERSION)" > bananui-base-$(VERSION)/.version
	(cd bananui-base-$(VERSION) && $(MAKE) clean)
	tar czf bananui-base_$(VERSION).orig.tar.gz bananui-base-$(VERSION)
	(cd bananui-base-$(VERSION) && debuild --no-lintian -us -uc \
		-aarmhf)

device-startup_$(VERSION)-1_all.deb: device-startup-$(VERSION)
	(cd device-startup-$(VERSION) && $(MAKE) clean)
	tar czf device-startup_$(VERSION).orig.tar.gz device-startup-$(VERSION)
	(cd device-startup-$(VERSION) && debuild --no-lintian -us -uc)

initrd.img: ramdisk
	rm -f $@
	./pack-initrd $@ $<

boot.img: initrd.img zImage bootimg.cfg
	abootimg --create $@ -f bootimg.cfg -k zImage -r $<

debroot:
	rm -rf debroot
	debootstrap --include=$(DEFAULT_PACKAGES) --arch armhf --foreign \
		stable debroot/ $(MIRROR)
	mkdir -p debroot/lib/modules/
	cp -rf modules debroot/lib/modules/3.10.49-bananian

debroot.tar: debroot $(DEBS)
	rm -f $@
	cp -f $(DEBS) debroot/var/cache
	(cd debroot; tar cvf ../$@ --exclude=.gitignore *)
	@echo "Now, you can push the files boot.img and debroot.tar to the" \
		"device and execute the following in your phone's root shell:"
	@echo " cd /data"
	@echo " mkdir debroot"
	@echo " busybox mount /dev/block/mmcblk0p2 debroot"
	@echo " cd debroot"
	@echo " busybox tar xvf /path/to/debroot.tar"
	@echo " mount -o bind /dev dev"
	@echo " mount -o bind /sys sys"
	@echo " mount -o bind /proc proc"
	@echo " export" \
	       "\"PATH=/usr/local/sbin:/usr/sbin:/usr/local/bin:/usr/bin:\$$PATH\""
	@echo " chroot . /bin/bash"
	@echo " debootstrap/debootstrap --second-stage"
	@echo " cd var/cache"
	@echo " dpkg -i $(DEBS)"
	@echo " rm $(DEBS)"
	@echo " exit"
	@echo " dd if=/path/to/boot.img" \
		"of=/dev/block/bootdevice/by-name/<recovery or boot> bs=2048"
	@echo " reboot recovery"

clean:
	rm -rf *.deb $(OUTPUTS)
