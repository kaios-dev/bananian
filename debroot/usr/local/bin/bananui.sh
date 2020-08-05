#!/bin/sh

/usr/local/bin/bananui > /dev/null < /dev/null 2>&1 &

jobs -p > /run/bananui.pid
