#!/bin/bash

# -----------------------------------------------------------------------------
#  This script can be used to make .bas executable by nuBASIC
# -----------------------------------------------------------------------------
#
#  This file is part of nuBASIC
#
#  nuBASIC is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  nuBASIC is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with nuBASIC; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  US
#
#  Author: Antonino Calderone <acaldmail@gmail.com>
# -----------------------------------------------------------------------------

file="$1"

if [ -z $file ]
then
   echo "Usage: $0 filename"
   exit 0
fi

if [ -f "$file" ]
then
	echo "$file found."
else
	echo "$file not found."
   exit 1
fi

# Remove Ctrl+M
dos2unix $1 2> /dev/null 

cp -a $1 $1.bak && echo "#!`which nubasic`" > $1 && grep -v "^#!" $1.bak >> $1 && chmod +x $1 || error=1

if [ -z $error ]
then
   echo "Operation completed"
   exit 0
else
   echo "Error making $1 executable"
   exit $error
fi

