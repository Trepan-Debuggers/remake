#!/bin/sh
#  $Id: autogen.sh,v 1.1 2004/06/11 00:03:31 rockyb Exp $
#  

# This needs to be run the first time you check this project out, 
# and possibly other times when things in CVS change drastically. 
# This rebuilds all the things that need rebuilding, installing
# missing files as symbolic links.

echo You may get warnings here about missing files like README, etc.
echo Ignore them, they are harmless.

autoreconf --install --symlink --verbose
