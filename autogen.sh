#!/bin/sh
aclocal -I config
autoheader
automake --add-missing
autoconf

