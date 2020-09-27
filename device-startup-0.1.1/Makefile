SCRIPTS = rc.wifi.sh rc.wifi.stop.sh
INITSCRIPTS = initscripts/wifi initscripts/fwload

all:

install:
	install $(INITSCRIPTS) $(DESTDIR)/etc/init.d
	install $(SCRIPTS) $(DESTDIR)/etc/
	install WCNSS_qcom_cfg.ini $(DESTDIR)/lib/firmware

clean:
