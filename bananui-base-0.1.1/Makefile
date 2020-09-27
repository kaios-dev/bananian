EXECS = bananui testclient mainclient browser colorgrid
HELPERS = bananui-bg
INITSCRIPTS = initscripts/bananui
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
LOGINSOURCES = login.c
LOGINOBJECTS = $(LOGINSOURCES:.c=.o)
SHUTDOWNSOURCES = shutdown.c
SHUTDOWNOBJECTS = $(SHUTDOWNSOURCES:.c=.0)
VERSION = $(shell cat .version)
CFLAGS = -g -Wall -Wno-unused-function -DHAVE_DEBUG -DVERSION="\"$(VERSION)\"" \
	-B "$$(pwd)/../debroot/usr/lib/arm-linux-gnueabihf/" \
	-B "$$(pwd)/../debroot/usr/include/arm-linux-gnueabihf/" \
	-B "$$(pwd)/../debroot/usr/include/" \
	-Wl,-rpath-link="$$(pwd)/../debroot/usr/lib/arm-linux-gnueabihf/"

all: $(EXECS)

bananui: $(UIOBJECTS)
	$(CC) -o $@ $(UIOBJECTS) $(CFLAGS)

install:
	install $(EXECS) $(HELPERS) $(DESTDIR)/usr/bin
	install $(INITSCRIPTS) $(DESTDIR)/etc/init.d
	install defconfig/* $(DESTDIR)/etc/bananui

testclient: $(TESTOBJECTS)
	$(CC) -o $@ $(TESTOBJECTS) $(CFLAGS)

mainclient: $(MAINCOBJECTS)
	$(CC) -o $@ $(MAINCOBJECTS) $(CFLAGS)

browser: $(BROWSEROBJECTS)
	$(CC) -o $@ $(BROWSEROBJECTS) $(CFLAGS)

colorgrid: $(COLORGOBJECTS)
	$(CC) -o $@ $(COLORGOBJECTS) $(CFLAGS)

bananui-login: $(LOGINOBJECTS)
	$(CC) -o $@ $(LOGINOBJECTS) $(CFLAGS) \
		$$(pwd)/../debroot/usr/lib/arm-linux-gnueabihf/libpam.so.0
	# -lpam does not work

bananui-shutdown: $(SHUTDOWNOBJECTS)
	$(CC) -o $@ $(SHUTDOWNOBJECTS) $(CFLAGS)

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $< -o $@

ifneq (clean, $(MAKECMDGOALS))
-include deps.mk
endif

deps.mk: $(UISOURCES)
	$(CC) -MM $^ > $@

clean:
	rm -f deps.mk *.o $(EXECS)
