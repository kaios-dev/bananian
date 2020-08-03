OUTPUTS = initrd.img boot.img debroot.tar
EXECS = bananui testclient mainclient browser colorgrid
UISOURCES = gr.c ui.c uiserv.c
UIOBJECTS = $(UISOURCES:.c=.o)
TESTSOURCES = testclient.c
TESTOBJECTS = $(TESTSOURCES:.c=.o)
MAINCSOURCES = mainclient.c
MAINCOBJECTS = $(MAINCSOURCES:.c=.o)
BROWSERSOURCES = browser.c
BROWSEROBJECTS = $(BROWSERSOURCES:.c=.o)
COLORGSOURCES = colorgrid.c
COLORGOBJECTS = $(COLORGSOURCES:.c=.o)
CFLAGS = -g -Wall -static -DHAVE_DEBUG

CC = $(shell ./check-deps findarmgcc)

all: check-deps $(EXECS) $(OUTPUTS)

check-deps:
	@./check-deps check
	@./check-deps checkgcc $(CC)

bananui: $(UIOBJECTS)
	$(CC) -o $@ $(UIOBJECTS) $(CFLAGS)

testclient: $(TESTOBJECTS)
	$(CC) -o $@ $(TESTOBJECTS) $(CFLAGS)

mainclient: $(MAINCOBJECTS)
	$(CC) -o $@ $(MAINCOBJECTS) $(CFLAGS)

browser: $(BROWSEROBJECTS)
	$(CC) -o $@ $(BROWSEROBJECTS) $(CFLAGS)

colorgrid: $(COLORGOBJECTS)
	$(CC) -o $@ $(COLORGOBJECTS) $(CFLAGS)

initrd.img: ramdisk
	rm -f $@
	./pack-initrd $@ $<

boot.img: initrd.img
	abootimg --create $@ -f bootimg.cfg -k zImage -r $<

debroot.tar: $(EXECS)
	rm -f $@
	cp $(EXECS) debroot/usr/local/bin
	tar cvf $@ --exclude=.gitignore debroot/

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $< -o $@

ifneq (clean, $(MAKECMDGOALS))
-include deps.mk
endif

deps.mk: $(UISOURCES)
	$(CC) -MM $^ > $@

clean:
	rm -f *.o $(EXECS) $(OUTPUTS)
