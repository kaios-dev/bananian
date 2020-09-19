SCRIPTS = rc.wifi.sh rc.wifi.stop.sh rc.usb.sh
INITSCRIPTS = initscripts/wifi initscripts/usb

all:

install:
	install $(INITSCRIPTS) $(DESTDIR)/etc/init.d
	install $(SCRIPTS) $(DESTDIR)/etc/

clean:
