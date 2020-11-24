#!/system/bin/sh

/data/run-in-debian.sh $@ 'debootstrap/debootstrap --second-stage; cd /var/cache && dpkg -i *.deb && rm -f *.deb && adduser --gecos "Bananian User" --disabled-password user && adduser user sudo && (echo user:bananian | chpasswd user)'
