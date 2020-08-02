OUTPUTS = initrd.img boot.img
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
CC = arm-linux-gnueabi-gcc-10

ifeq (desktop, $(TARGET))
CFLAGS += -DDESKTOP
endif

all: $(EXECS) $(OUTPUTS)

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

initrd.img:
	abootimg-pack-initrd

boot.img: initrd.img
	abootimg --create boot.img -f bootimg.cfg -k zImage -r initrd.img
%.o: %.c %.h
	$(CC) $(CFLAGS) -c $< -o $@

ifneq (clean, $(MAKECMDGOALS))
-include deps.mk
endif

deps.mk: $(UISOURCES)
	$(CC) -MM $^ > $@

clean:
	rm -f *.o $(EXECS) $(OUTPUTS)
