#!/bin/sh
#  $Id: autogen.sh,v 1.3 2006/01/09 05:35:08 rockyb Exp $
echo "Invoking automake explicitly because autoreconf doesn't find config/elisp-comp"
set -x
automake --add-missing
autoreconf --install $*
