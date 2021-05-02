#!/system/bin/sh

echo "[on device] ==>> Bootstrapping debian"
/data/run-in-debian.sh $@ 'debootstrap/debootstrap --second-stage; cd /var/cache && dpkg -i *.deb && rm -f *.deb && adduser --gecos "Bananian User" --disabled-password user && adduser user sudo && addgroup --system ofono && adduser user ofono && adduser user audio && (echo user:bananian | chpasswd user)'
