# This is a maintainer's-only makefile
#
# It bootstraps a GNU make maintainer's directory
#

.SUFFIXES:

NORECURSE = true

# If the user asked for a specific target, invoke the Mkaefile instead.
#
.DEFAULT:
	@[ -f Makefile.in -a -f configure -a -f aclocal.m4 -a -f config.h.in ] \
	  || $(MAKE) __cfg NORECURSE=
	@[ -f Makefile ] \
	  || ./configure
	$(MAKE) -f Makefile $@

.PHONY: __cfg __cfg_basic TAGS

# This is variable since the glob subdirectory doesn't use it.
#
ACCONFIG = acconfig.h

__cfg: __cfg_basic config.h.in TAGS
	cd glob && $(MAKE) -f ../GNUmakefile __cfg_basic ACCONFIG=
ifdef NORECURSE
	@echo ""; echo "Now you should run one of:"; echo ""; \
	  echo "  make all"; \
	  echo "  make dist"; \
	  echo "  make distdir"; \
	  echo "  make distcheck"; echo ""; \
	  echo "Or similar to proceed.";\
	  echo ""
endif

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

TAGS:
	find . -name '*.[ch]' -print | etags -
