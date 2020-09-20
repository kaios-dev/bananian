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
VERSION = $(shell cat .version)
CFLAGS = -g -Wall -Wno-unused-function -DHAVE_DEBUG -DVERSION="\"$(VERSION)\""

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

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $< -o $@

ifneq (clean, $(MAKECMDGOALS))
-include deps.mk
endif

deps.mk: $(UISOURCES)
	$(CC) -MM $^ > $@

clean:
	rm -f *.o $(EXECS)
