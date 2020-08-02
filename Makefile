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

all: $(EXECS)

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

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $< -o $@

ifneq (clean, $(MAKECMDGOALS))
-include deps.mk
endif

deps.mk: $(UISOURCES)
	$(CC) -MM $^ > $@

clean:
	rm -f *.o $(EXECS)
