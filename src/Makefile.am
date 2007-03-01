## $Id: Makefile.am,v 1.22 2007/03/01 16:10:59 rockyb Exp $
## Process this file with automake to create Makefile.in.
## Copyright 2002, 2004, 2005, 2006 Free Software Foundation, Inc.
## This is a -*-Makefile-*-, or close enough

transform := s,make,@MAKE_NAME@,

INCLUDES := -I$(top_srcdir)
ACLOCAL_AMFLAGS	=  -I $(top_srcdir)/config

bin_PROGRAMS :=	make

getopt_sources := getopt.c getopt1.c getopt.h

if USE_CUSTOMS
  remote :=	remote-cstms.c
else
  remote :=	remote-stub.c
endif

if USE_GETOPT
   add_getopt_sources := $(getopt_sources)
else
   add_getopt_sources :=
endif

make_SOURCES :=	ar_fns.c arscan.c buildargv.c commands.c \
	        dbg_break.c dbg_cmd.c dbg_fns.c dbg_stack.c \
	        debug.c default.c dep.c dir_fns.c expand.c file.c \
		function.c hash.c implicit.c job.c \
	        main.c misc.c print.c read.c remake.c $(remote) rule.c \
	        signame.c strcache.c trace.c variable.c version.c vpath.c \
	        $(add_getopt_sources) 

EXTRA_make_SOURCES := remote-stub.c remote-cstms.c $(getopt_sources)

noinst_HEADERS := ar_fns.h arscan.h buildargv.h commands.h \
	         dbg_break.h dbg_cmd.h dbg_fns.h dbg_stack.h \
	         debug.h default.h dep.h dir_fns.h expand.h file.h \
	         function.h getopt.h gettext.h hash.h implicit.h job.h \
		 make.h misc.h print.h \
	         read.h remake.h remote-stub.h rule.h \
		 strcache.h trace.h types.h variable.h  vpath.h

make_LDADD :=	@LIBOBJS@ @ALLOCA@ $(GLOBLIB) @GETLOADAVG_LIBS@ @LIBINTL@ \
	        @LIBGETOPT_LIB@

DEFS =		-DLOCALEDIR=\"$(localedir)\" -DLIBDIR=\"$(libdir)\" -DINCLUDEDIR=\"$(includedir)\" @DEFS@

AM_CPPFLAGS :=	$(GLOBINC)


MAKE_HOST :=	@MAKE_HOST@

# --------------- Internationalization Section

localedir =     $(datadir)/locale


.PHONY: check-loadavg 

check-loadavg: loadavg$(EXEEXT)
	@echo The system uptime program believes the load average to be:
	-uptime
	@echo The GNU load average checking code thinks:
	-./loadavg$(EXEEXT)

# The loadavg function is invoked during "make check" to test getloadavg.
check_PROGRAMS := loadavg
loadavg_CPPFLAGS := -DTEST
loadavg_LDADD := @GETLOADAVG_LIBS@

loadavg.o: loadavg.c
	source='$<' object='$@' libtool=no \
	depfile='$(DEPDIR)/$*.Po' tmpdepfile='$(DEPDIR)/$*.TPo' \
	$(CCDEPMODE) $(depcomp) \
	$(COMPILE) -DTEST -c `test -f '$<' || echo '$(srcdir)/'`$<



# Extra stuff to include in the distribution.
EXTRA_DIST :=	

# Forward targets

# --------------- Local CHECK Section

check-local: check-loadavg

# check-regression
#
# Look for the make test suite, and run it if found and we can find perl.
# If we're building outside the tree, we use symlinks to make a local copy of
# the test suite.  Unfortunately the test suite itself isn't localizable yet.
#
MAKETESTFLAGS :=

# --------------- Maintainer's Section

@MAINT_MAKEFILE@
