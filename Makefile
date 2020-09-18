VERSION = 0.0.2
OUTPUTS = initrd.img boot.img debroot debroot.tar
DEBS = bananui-base_$(VERSION)-1_armhf.deb

CC = $(shell ./check-deps findarmgcc $(CROSS_COMPILE))

all: check-deps $(OUTPUTS)

check-deps::
	@./check-deps check
	@./check-deps checkgcc $(CC)

bananui-base_$(VERSION)-1_armhf.deb:
	(cd bananui-base-$(VERSION) && $(MAKE) clean)
	tar czf bananui-base_$(VERSION).orig.tar.gz bananui-base-$(VERSION)
	(cd bananui-base-$(VERSION) && debuild -us -uc)

initrd.img: ramdisk
	rm -f $@
	./pack-initrd $@ $<

boot.img: initrd.img zImage bootimg.cfg
	abootimg --create $@ -f bootimg.cfg -k zImage -r $<

debroot: $(DEBS)

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
	rm -rf *.o $(DEBS) $(OUTPUTS)
