#!/bin/sh
### autogen.sh with sensible comments ###############################

# Use this script to bootstrap your build AFTER checking it out from
# source control. You should not have to use it for anything else.

### AUTOTOOLS #######################################################
# Runs autoconf, autoheader, aclocal, automake, autopoint, libtoolize
echo "Regenerating autotools files"
autoreconf --install --force --symlink || exit 1

echo "Now run ./configure, make, and make install."

