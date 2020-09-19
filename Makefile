VERSION = 0.0.2
OUTPUTS = initrd.img boot.img debroot debroot.tar
DEFAULT_PACKAGES = openssh-server,vim,wpasupplicant,man-db
DEBS = bananui-base_$(VERSION)-1_armhf.deb device-startup_$(VERSION)-1_all.deb

CC = $(shell ./check-deps findarmgcc $(CROSS_COMPILE))

all: check-deps $(OUTPUTS)

check-deps::
	@./check-deps check
	@./check-deps checkgcc $(CC)
	@echo "$(CC)" > .target-gcc

bananui-base_$(VERSION)-1_armhf.deb: bananui-base-$(VERSION)
	(cd bananui-base-$(VERSION) && $(MAKE) clean)
	tar czf bananui-base_$(VERSION).orig.tar.gz bananui-base-$(VERSION)
	(cd bananui-base-$(VERSION) && debuild -us -uc)

device-startup_$(VERSION)-1_all.deb: device-startup-$(VERSION)
	(cd device-startup-$(VERSION) && $(MAKE) clean)
	tar czf device-startup_$(VERSION).orig.tar.gz device-startup-$(VERSION)
	(cd device-startup-$(VERSION) && debuild -us -uc)

initrd.img: ramdisk
	rm -f $@
	./pack-initrd $@ $<

boot.img: initrd.img zImage bootimg.cfg
	abootimg --create $@ -f bootimg.cfg -k zImage -r $<

debroot: $(DEBS)
	rm -rf debroot
	debootstrap --include=$(DEFAULT_PACKAGES) --arch armhf stable debroot/ $(MIRROR)
	dpkg --root debroot/ -i $(DEBS)

debroot.tar:
#	rm -f $@
#	cp $(EXECS) debroot/usr/local/bin
#	(cd debroot; chmod a=rwxt tmp; tar cvf ../$@ --owner=0 --exclude=.gitignore *)

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $< -o $@

ifneq (clean, $(MAKECMDGOALS))
-include deps.mk
endif

deps.mk: $(UISOURCES)
	$(CC) -MM $^ > $@

clean:
	rm -rf .target-gcc *.o *.deb $(OUTPUTS)
