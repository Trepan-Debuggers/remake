#!/bin/sh
#  $Id: autogen.sh,v 1.4 2006/01/09 06:34:59 rockyb Exp $
# Tried autoreconf. It didn't work. 
#   --install didn't add --add-missing to automake
#   and didn't invocate autopoint and autoheader 
# I'm sure it must somehow my fault and not autoreconf's.
# So for now don't use autoreconf.
set -x
autopoint
aclocal -I . -I config
autoheader
automake --add-missing
autoconf
