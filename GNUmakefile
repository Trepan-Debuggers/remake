# This is a maintainer's-only makefile
#
# It bootstraps a GNU make maintainer's directory
#

.SUFFIXES:

NORECURSE = true

ACLOCALARGS =

CFLAGS = -g -O -Wall -D__USE_FIXED_PROTOTYPES__
export CFLAGS

# If the user asked for a specific target, invoke the Makefile instead.
#
.DEFAULT:
	@[ -f Makefile.in -a -f configure -a -f aclocal.m4 -a -f config.h.in ] \
	  || $(MAKE) __cfg NORECURSE=
	@[ -f Makefile ] \
	  || CFLAGS='-g -O -Wall -D__USE_FIXED_PROTOTYPES__' ./configure
	$(MAKE) -f Makefile $@

.PHONY: __cfg __cfg_basic

# This is variable since the glob subdirectory doesn't use it.
#
ACCONFIG = acconfig.h

__cfg: __cfg_basic config.h.in TAGS
ifdef NORECURSE
	@echo ""; echo "Now you should run:"; echo ""; \
	  echo "  make all"; echo ""; \
	  echo "then, optionally, one of:"; echo ""; \
	  echo "  make dist"; \
	  echo "  make distdir"; \
	  echo "  make distcheck"; \
	  echo ""
endif

__cfg_basic: aclocal.m4 stamp-h.in configure Makefile.in

aclocal.m4: configure.in $(wildcard acinclude.m4)
	aclocal $(ACLOCALARGS)

config.h.in: stamp-h.in
stamp-h.in: configure.in aclocal.m4 $(ACCONFIG)
	autoheader
	echo timestamp > $@

configure: configure.in aclocal.m4
	autoconf $(ACARGS)

Makefile.in: configure.in config.h.in Makefile.am aclocal.m4
	automake --add-missing

TAGS:
	find . -name '*.[ch]' -print | etags -
