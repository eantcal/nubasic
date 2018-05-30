#!/bin/sh
./configure --enable-gtkide
sudo checkinstall --pkgrelease=1.48 --pkgversion=1 --strip=yes --stripso=yes --addso=yes --pkglicense="MIT" -y --maintainer="Antonino Calderone antonino.calderone@gmail.com" make install
