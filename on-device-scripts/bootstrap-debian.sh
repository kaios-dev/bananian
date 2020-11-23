#!/system/bin/sh

/data/run-in-debian.sh $@ /bin/sh -c \''debootstrap/debootstrap --second-stage; cd /var/cache && dpkg -i *.deb && rm -f *.deb && adduser --gecos "Bananian User" --disabled-password user && adduser user sudo && passwd -d -e user'\'
