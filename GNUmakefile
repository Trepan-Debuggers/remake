# This is a maintainer's-only makefile
#
# It bootstraps a GNU make maintainer's directory
#

.SUFFIXES:

.DEFAULT:
	[ -f Makefile ] || ./configure
	$(MAKE) -f Makefile $@

.PHONY: __cfg __cfg_basic

ACCONFIG = acconfig.h

__cfg: __cfg_basic config.h.in
	cd glob && $(MAKE) -f ../GNUmakefile __cfg_basic ACCONFIG=

__cfg_basic: aclocal.m4 stamp-h.in configure Makefile.in

aclocal.m4: configure.in
	aclocal

config.h.in: stamp-h.in
stamp-h.in: configure.in aclocal.m4 $(ACCONFIG)
	autoheader
	echo timestamp > $@

configure: configure.in aclocal.m4
	autoconf

Makefile.in: configure.in config.h.in Makefile.am aclocal.m4
	automake --add-missing
