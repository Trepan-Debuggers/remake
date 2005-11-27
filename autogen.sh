#!/bin/sh
#  $Id: autogen.sh,v 1.2 2005/11/27 12:32:17 rockyb Exp $
#  

# This needs to be run the first time you check this project out, 
# and possibly other times when things in CVS change drastically. 
# This rebuilds all the things that need rebuilding, installing
# missing files as symbolic links.

echo You may get warnings here about missing files like README, etc.
echo Ignore them, they are harmless.

srcdir=`dirname $0`

autoreconf --install --symlink --verbose

conf_flags="--enable-maintainer-mode" # --enable-compile-warnings #--enable-iso-c

if test x$NOCONFIGURE = x; then
  echo Running $srcdir/configure $conf_flags "$@" ...
  $srcdir/configure $conf_flags "$@" \
  && echo Now type \`make\' to compile $PKG_NAME
else
  echo Skipping configure process.
fi

# FIXME: create po/*.po so we can remove them from CVS.
