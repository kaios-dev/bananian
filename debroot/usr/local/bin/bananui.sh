#!/bin/sh

rm -f /tmp/bananui.sock
/usr/local/bin/bananui > /var/log/bananui.log < /dev/null 2>&1 &
jobs -p > /run/bananui.pid
