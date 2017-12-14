#!/bin/bash
set -e
./autogen.sh --with-libusb --prefix=/home/user/urjtag-install
make && make install
