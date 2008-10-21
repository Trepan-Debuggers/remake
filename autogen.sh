#!/bin/sh
aclocal -I config
autoheader
automake
autoconf

